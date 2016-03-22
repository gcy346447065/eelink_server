/*
 * msg_proc_simcom.c
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */


#include <string.h>
#include <netinet/in.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include <object.h>

#include "msg_proc_simcom.h"
#include "protocol.h"
#include "log.h"
#include "object.h"
#include "msg_simcom.h"
#include "db.h"
#include "mqtt.h"
#include "cJSON.h"
#include "yunba_push.h"
#include "msg_app.h"
#include "cgi2gps.h"
#include "sync.h"
#include "firmware_upgrade.h"

typedef int (*MSG_PROC)(const void *msg, SESSION *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int simcom_sendMsg(void *msg, size_t len, SESSION *session)
{
    if (!session)
    {
        return -1;
    }

    MSG_SEND pfn = session->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        return -1;
    }
    pfn(session->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_msg_cmd(msg), len);
    LOG_HEX(msg, len);
    free_simcom_msg(msg);

    return 0;
}

static time_t get_time()
{
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}

static int simcom_wild(const void *m, SESSION *session)
{
	const MSG_HEADER *msg = m;
    const char *msg_log = (const char *)(msg + 1);

    LOG_DEBUG("DBG:%s", msg_log);
	app_sendDebugMsg2App(msg_log, ntohs(msg->length), session);

	return 0;
}

static int simcom_login(const void *msg, SESSION *session)
{
    const MSG_LOGIN_REQ *req = (const MSG_LOGIN_REQ *)msg; //TO DO: Version, CCID
    char imei[IMEI_LENGTH];
    memcpy(imei, req->IMEI, IMEI_LENGTH);

    if (!session)
    {
        LOG_FATAL("session ptr null");
        return -1;
    }

    OBJECT * obj = session->obj;
    if(!obj)
    {
        LOG_DEBUG("mc IMEI(%s) login", imei);

        obj = obj_get(imei);
        if (!obj)
        {
            LOG_INFO("the first time of simcom IMEI(%s)'s login", imei);

            //if there is no data uploaded in 10 min, the device will become offline
            //for the 600s timeout of connection in server_simcom.c

            obj = obj_new();

            memcpy(obj->IMEI, imei, IMEI_LENGTH);
            memcpy(obj->DID,  imei, IMEI_LENGTH);//IMEI and DID mean the same now
            obj->ObjectType = ObjectType_simcom;

            obj_add(obj);

            sync_newIMEI(obj->IMEI);
            mqtt_subscribe(obj->IMEI);
        }

        session->obj = obj;
        session_add(session);
        obj->session = session;
    }
    else
    {
        LOG_INFO("simcom IMEI(%s) already login", imei);
    }

    //login rsp
    MSG_LOGIN_RSP *rsp = alloc_simcom_rspMsg((const MSG_HEADER *)msg);
    if(rsp)
    {
        simcom_sendMsg(rsp, sizeof(MSG_LOGIN_RSP), session);
        LOG_INFO("send login rsp");
    }
    else
    {
        free(rsp);
        LOG_ERROR("insufficient memory");
        return -1;
    }

    int ret = 0;
    if(!db_isTableCreated(obj->IMEI, &ret) && !ret)
    {
        LOG_INFO("create tables of %s", obj->IMEI);
        db_createCGI(obj->IMEI);
        db_createGPS(obj->IMEI);
    }

    //get version, compare the version number; if not, send upgrade start message
    int theLastVersion = 0, theSize = 0;
    if(theLastVersion = getLastVersionWithFileNameAndSizeStored())
    {
        theSize = getLastFileSize();
        LOG_INFO("req->version is %d, theLastVersion is %d, theSize is %d", ntohl(req->version), theLastVersion, theSize);
        
        if(ntohl(req->version) < theLastVersion)
        {
            MSG_UPGRADE_START_REQ *req4upgrade = (MSG_UPGRADE_START_REQ *)alloc_simcomUpgradeStartReq(theLastVersion, theSize);
            if (!req4upgrade)
            {
                LOG_FATAL("insufficient memory");
            }

            simcom_sendMsg(req4upgrade, sizeof(MSG_UPGRADE_START_REQ), session);
        }
    }
    else
    {
        LOG_ERROR("can't get valid theLastVersion");
    }

    return 0;
}

static int simcom_ping(const void *msg, SESSION *session)
{
    //TO DO: unused
    //const MSG_PING_REQ *req = (const MSG_PING_REQ *)msg;
    //ntohs(req->status);

    if (!session)
    {
        LOG_FATAL("session ptr null");
        return -1;
    }

    OBJECT * obj = (OBJECT *)session->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    LOG_INFO("imei(%s) ping", obj->IMEI);

    return 0;
}

static int simcom_gps(const void *msg, SESSION *session)
{
    const MSG_GPS *req = (const MSG_GPS *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->header.length) < sizeof(MSG_GPS) - MSG_HEADER_LEN)
    {
        LOG_ERROR("gps message length not enough");
        return -1;
    }

    LOG_INFO("GPS: timestamp(%d), latitude(%f), longitude(%f), speed(%d), course(%d)",
        ntohl(req->gps.timestamp), req->gps.latitude, req->gps.longitude, req->gps.speed, ntohs(req->gps.course));

    OBJECT * obj = (OBJECT *)session->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    obj->timestamp = ntohl(req->gps.timestamp);
    obj->lat = req->gps.latitude;
    obj->lon = req->gps.longitude;
    obj->speed = req->gps.speed;
    obj->course = ntohs(req->gps.course);
    obj->isGPSlocated = 0x01;

    app_sendGpsMsg2App(session);

    db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);
    sync_gps(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);

    return 0;
}

static int simcom_cell(const void *msg, SESSION *session)
{
    const MSG_HEADER *req = (const MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->length) < sizeof(CGI))
    {
        LOG_ERROR("cell message length not enough");
        return -1;
    }

    const CGI *cgi = (const CGI *)(req + 1);
    if (!cgi)
    {
        LOG_ERROR("cgi handle empty");
        return -1;
    }

    LOG_INFO("CGI: mcc(%d), mnc(%d)", ntohs(cgi->mcc), ntohs(cgi->mnc));

    OBJECT *obj = session->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    obj->timestamp = get_time();
    obj->isGPSlocated = 0x00;

    int num = (int)cgi->cellNo;
    if(num > CELL_NUM)
    {
        LOG_ERROR("Number:%d of cell is over", num);
        return -1;
    }

    const CELL *cell = (const CELL *)(cgi + 1);
    if (!cell)
    {
        LOG_ERROR("cell handle empty");
        return -1;
    }

    for(int i = 0; i < num; ++i)
    {
        (obj->cell[i]).mcc = ntohs(cgi->mcc);
        (obj->cell[i]).mnc = ntohs(cgi->mnc);
        (obj->cell[i]).lac = ntohs((cell[i]).lac);
        (obj->cell[i]).ci  = ntohs((cell[i]).cellid);
        (obj->cell[i]).rxl = ntohs((cell[i]).rxl);
    }
    db_saveCGI(obj->IMEI, obj->timestamp, obj->cell, num);

#if 0
    float lat, lon;
    int rc = cgi2gps(obj->cell, num, &lat, &lon);
    if(rc != 0)
    {
        //LOG_ERROR("cgi2gps error");
        return 1;
    }
    obj->lat = lat;
    obj->lon = lon;
    obj->altitude = 0;
    obj->speed = 0;
    obj->course = 0;

    app_sendGpsMsg2App(session);
    db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, 0, 0);
#endif

    return 0;
}

static int simcom_alarm(const void *msg, SESSION *session)
{
    const MSG_ALARM_REQ *req = (const MSG_ALARM_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->header.length) < sizeof(MSG_ALARM_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("alarm message length not enough");
        return -1;
    }

    LOG_INFO("ALARM: %d", req->alarmType);
    OBJECT *obj = session->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    //send to APP by MQTT
    app_sendAlarmMsg2App(req->alarmType, NULL, session);

    //send to APP by yunba
    char topic[128];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, 128, "simcom_%s", obj->IMEI);

    cJSON *root = cJSON_CreateObject();
    cJSON *alarm = cJSON_CreateObject();
    cJSON_AddNumberToObject(alarm, "type", req->alarmType);
    cJSON_AddItemToObject(root, "alarm", alarm);
    char* json = cJSON_PrintUnformatted(root);

    yunba_publish(topic, json, strlen(json));
    LOG_INFO("send alarm: %s", topic);

    free(json);
    cJSON_Delete(root);

    return 0;
}

static int simcom_sms(const void *msg , SESSION *session)
{
    const MSG_SMS_REQ *req = (const MSG_SMS_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->header.length) < sizeof(MSG_SMS_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("sms message length not enough");
        return -1;
    }

    LOG_INFO("SMS: %s", req->telphone);

    //TO DO: sms

    return 0;
}

static int simcom_433(const void *msg, SESSION *session)
{
    const MSG_433_REQ *req = (const MSG_433_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->header.length) < sizeof(MSG_433_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("433 message length not enough");
        return -1;
    }

    LOG_INFO("433: %d", ntohl(req->intensity));

    OBJECT *obj = session->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    app_send433Msg2App(get_time(), ntohl(req->intensity), session);
    return 0;
}

static int simcom_defend(const void *msg, SESSION *session)
{
    const MSG_DEFEND_RSP *rsp = (const MSG_DEFEND_RSP *)msg;
    int defend = ntohl(rsp->token);

    OBJECT* obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(defend == APP_CMD_FENCE_ON)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_SUCCESS, strIMEI);
        }
    }
    else if(defend == APP_CMD_FENCE_OFF)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_SUCCESS, strIMEI);
        }
    }
    else if(defend == APP_CMD_FENCE_GET)
    {
        if(rsp->result == DEFEND_ON)
        {
            app_sendFenceGetRsp2App(APP_CMD_FENCE_GET, CODE_SUCCESS, 1, session);
        }
        else if(rsp->result == DEFEND_OFF)
        {
            app_sendFenceGetRsp2App(APP_CMD_FENCE_GET, CODE_SUCCESS, 0, session);
        }
    }
    else
    {
        LOG_ERROR("response defend cmd error");
        return -1;
    }

    return 0;
}

static int simcom_seek(const void *msg, SESSION *session)
{
    const MSG_SEEK_RSP *rsp = (const MSG_SEEK_RSP *)msg;
    int seek = ntohl(rsp->token);

    OBJECT* obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(seek == APP_CMD_SEEK_ON)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_SEEK_ON, CODE_SUCCESS, strIMEI);
        }
    }
    else if(seek == APP_CMD_SEEK_OFF)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_SEEK_OFF, CODE_SUCCESS, strIMEI);
        }
    }
    else
    {
        LOG_ERROR("response seek cmd not exist");
        return -1;
    }

    return 0;
}

static int simcom_locate(const void *msg, SESSION *session)
{
    const MSG_HEADER *req = (const MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("req handle empty");
        return -1;
    }
    if(ntohs(req->length) < sizeof(CGI) && ntohs(req->length) < sizeof(GPS))
    {
        LOG_ERROR("location message length not enough");
        return -1;
    }

    const char *isGPS = (const char *)(req + 1);

    if(*isGPS == 0x01)
    {
        //location with gps
        const GPS *gps = (const GPS *)(isGPS + 1);
        if (!gps)
        {
            LOG_ERROR("gps handle empty");
            return -1;
        }

        LOG_INFO("LOCATION GPS: timestamp(%d), latitude(%f), longitude(%f), speed(%d), course(%d)",
            ntohl(gps->timestamp), gps->latitude, gps->longitude, gps->speed, ntohs(gps->course));

        OBJECT * obj = (OBJECT *) session->obj;
        if (!obj)
        {
            LOG_WARN("MC must first login");
            return -1;
        }

        obj->timestamp = ntohl(gps->timestamp);
        obj->isGPSlocated = 0x01;
        obj->lat = gps->latitude;
        obj->lon = gps->longitude;
        obj->speed = gps->speed;
        obj->course = ntohs(gps->course);

        app_sendLocationRsp2App(CODE_SUCCESS, obj);
    }
    else if(*isGPS == 0x00)
    {
        //location with cell
        const CGI *cgi = (const CGI *)(isGPS + 1);
        if (!cgi)
        {
            LOG_ERROR("cgi handle empty");
            return -1;
        }

        LOG_INFO("LOCATION CGI: mcc(%d), mnc(%d)", ntohs(cgi->mcc), ntohs(cgi->mnc));
        OBJECT *obj = session->obj;
        if(!obj)
        {
            LOG_WARN("MC must first login");
            return -1;
        }

        obj->timestamp = get_time();
        obj->isGPSlocated = 0x00;

        int num = cgi->cellNo;
        if(num > CELL_NUM)
        {
            LOG_ERROR("Number:%d of cell is over", num);
            return -1;
        }

        const CELL *cell = (const CELL *)(cgi + 1);

        for(int i = 0; i < num; ++i)
        {
            (obj->cell[i]).mcc = ntohs(cgi->mcc);
            (obj->cell[i]).mnc = ntohs(cgi->mnc);
            (obj->cell[i]).lac = ntohs((cell[i]).lac);
            (obj->cell[i]).ci  = ntohs((cell[i]).cellid);
            (obj->cell[i]).rxl = ntohs((cell[i]).rxl);
        }

        float lat, lon;
        int rc = cgi2gps(obj->cell, num, &lat, &lon);
        if(rc != 0)
        {
            //LOG_ERROR("cgi2gps error");
            return 1;
        }
        obj->lat = lat;
        obj->lon = lon;
        obj->speed = 0;
        obj->course = 0;

        app_sendLocationRsp2App(CODE_SUCCESS, obj);
    }

    return 0;
}

static int simcom_SetTimer(const void *msg, SESSION *session)
{
    const MSG_GPSTIMER_RSP *rsp = (const MSG_GPSTIMER_RSP *)msg;

    OBJECT* obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }

    //TO DO: add set timer in APP first
    if(ntohl(rsp->result) == 0)
    {
        //APP_CMD_SET_TIMER, CODE_SUCCESS
        //app_sendCmdRsp2App(APP_CMD_SEEK_ON, CODE_SUCCESS, strIMEI);
    }
    else if(ntohl(rsp->result) >= 10)
    {
        //APP_CMD_GET_TIMER, CODE_SUCCESS
    }
    else
    {
        //APP_CMD_SET_TIMER, CODE_INTERNAL_ERR?
        return -1;
    }

    return 0;
}

static int simcom_SetAutoswitch(const void *msg, SESSION *session)
{
    const MSG_AUTOLOCK_SET_RSP *rsp = (const MSG_AUTOLOCK_SET_RSP *)msg;
    int autolock = ntohl(rsp->token);

    OBJECT* obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(autolock == APP_CMD_AUTOLOCK_ON)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_AUTOLOCK_ON, CODE_SUCCESS, strIMEI);
        }
    }
    else if(autolock == APP_CMD_AUTOLOCK_OFF)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_AUTOLOCK_OFF, CODE_SUCCESS, strIMEI);
        }
    }
    else
    {
        LOG_ERROR("response SetAutoswitch cmd not exist");
        return -1;
    }

    return 0;
}

static int simcom_GetAutoswitch(const void *msg, SESSION *session)
{
    const MSG_AUTOLOCK_GET_RSP *rsp = (const MSG_AUTOLOCK_GET_RSP *)msg;

    OBJECT* obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(ntohl(rsp->token) == APP_CMD_AUTOLOCK_GET)
    {
        if(rsp->result == 0 || rsp->result == 1)
        {
            app_sendAutoLockGetRsp2App(APP_CMD_AUTOLOCK_GET, CODE_SUCCESS, rsp->result, session);
        }
    }

    return 0;
}

static int simcom_SetPeriod(const void *msg, SESSION *session)
{
    const MSG_AUTOPERIOD_SET_RSP *rsp = (const MSG_AUTOPERIOD_SET_RSP *)msg;

    OBJECT *obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(ntohl(rsp->token) == APP_CMD_AUTOPERIOD_SET)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_AUTOPERIOD_SET, CODE_SUCCESS, strIMEI);
        }
    }
    else
    {
        LOG_ERROR("response SetPeriod cmd not exist");
        return -1;
    }
    return 0;
}

static int simcom_GetPeriod(const void *msg, SESSION *session)
{
    const MSG_AUTOPERIOD_GET_RSP *rsp = (const MSG_AUTOPERIOD_GET_RSP *)msg;
    int period = rsp->result;

    if(ntohl(rsp->token) == APP_CMD_AUTOPERIOD_GET)
    {
        if(period > 0)
        {
            app_sendAutoPeriodGetRsp2App(APP_CMD_AUTOPERIOD_GET, CODE_SUCCESS, period, session);
        }
    }
    else
    {
        LOG_ERROR("response GetPeriod cmd not exist");
        return -1;
    }

    return 0;
}

static int simcom_itinerary(const void *msg, SESSION *session)
{
    const MSG_ITINERARY_REQ *req = (const MSG_ITINERARY_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->header.length) < sizeof(MSG_ITINERARY_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("itinerary message length not enough");
        return -1;
    }

    LOG_INFO("itinerary: start(%d), end(%d), miles(%d)", ntohl(req->start), ntohl(req->end), ntohl(req->miles));

    OBJECT *obj = session->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    sync_itinerary(obj->IMEI, ntohl(req->start), ntohl(req->end), ntohl(req->miles));

    return 0;
}

static int simcom_battery(const void *msg, SESSION *session)
{
    const MSG_BATTERY_RSP *rsp = (const MSG_BATTERY_RSP *)msg;

    if(rsp->percent != 0)
    {
        LOG_INFO("battery: percent(%d), miles(%d)", rsp->percent, rsp->miles);
        app_sendBatteryRsp2App(APP_CMD_BATTERY, CODE_SUCCESS, rsp->percent, rsp->miles, session);
    }
    else
    {
        LOG_INFO("battery: percent(%d), miles(%d), it's learning now", rsp->percent, rsp->miles);
        app_sendBatteryRsp2App(APP_CMD_BATTERY, CODE_BATTERY_LEARNING, rsp->percent, rsp->miles, session);

        return -1;
    }

    return 0;
}

static int simcom_DefendOn(const void *msg, SESSION *session)
{
    const MSG_DEFEND_ON_RSP *rsp = (const MSG_DEFEND_ON_RSP *)msg;

    OBJECT *obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(rsp->result == 0)
    {
        app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_SUCCESS, strIMEI);
    }
    else
    {
        app_sendCmdRsp2App(APP_CMD_FENCE_ON, CODE_INTERNAL_ERR, strIMEI);
    }

    return 0;
}

static int simcom_DefendOff(const void *msg, SESSION *session)
{
    const MSG_DEFEND_OFF_RSP *rsp = (const MSG_DEFEND_OFF_RSP *)msg;

    OBJECT *obj = session->obj;
    if (!obj)
    {
        LOG_FATAL("internal error: obj null");
        return -1;
    }
    const char *strIMEI = obj->IMEI;

    if(rsp->result == 0)
    {
        app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_SUCCESS, strIMEI);
    }
    else
    {
        app_sendCmdRsp2App(APP_CMD_FENCE_OFF, CODE_INTERNAL_ERR, strIMEI);
    }

    return 0;
}

static int simcom_DefendGet(const void *msg, SESSION *session)
{
    const MSG_DEFEND_GET_RSP *rsp = (const MSG_DEFEND_GET_RSP *)msg;

    if(rsp->status == 0 || rsp->status == 1)
    {
        app_sendFenceGetRsp2App(APP_CMD_FENCE_GET, CODE_SUCCESS, rsp->status, session);
    }
    else
    {
        LOG_ERROR("response DefendGet cmd not exist");
        return -1;
    }

    return 0;
}

static int simcom_DefendNotify(const void *msg, SESSION *session)
{
    const MSG_DEFEND_NOTIFY_RSP *rsp = (const MSG_DEFEND_NOTIFY_RSP *)msg;

    if(rsp->status == 0 || rsp->status == 1)
    {
        app_sendNotifyMsg2App(NOTIFY_AUTOLOCK, get_time(), rsp->status, session);
    }
    else
    {
        LOG_ERROR("response DefendNotify cmd not exist");
        return -1;
    }

    return 0;
}

static int simcom_UpgradeStart(const void *msg, SESSION *session)
{
    const MSG_UPGRADE_START_RSP *rsp = (const MSG_UPGRADE_START_RSP *)msg;

    if(rsp->code == 0)
    {
        LOG_INFO("response get upgrade start rsp ok");

        char data[1024];
        int size;
        getDataSegmentWithGottenSize(0, data, &size);

        MSG_UPGRADE_DATA_REQ *req = (MSG_UPGRADE_DATA_REQ *)alloc_simcomUpgradeDataReq(0, data, size);
        if (!req)
        {
            LOG_FATAL("insufficient memory");
        }

        simcom_sendMsg(req, sizeof(MSG_UPGRADE_DATA_REQ) + size, session);
    }
    else
    {
        LOG_INFO("response get upgrade start code(%d) error", rsp->code);
    }

    return 0;
}

static int simcom_UpgradeData(const void *msg, SESSION *session)
{
    const MSG_UPGRADE_DATA_RSP *rsp = (const MSG_UPGRADE_DATA_RSP *)msg;

    if(ntohl(rsp->size) > 0)
    {
        int LastSize = getLastFileSize();
        LOG_INFO("rsp->size is %d, LastSize is %d", ntohl(rsp->size), LastSize);

        if(ntohl(rsp->size) < LastSize)
        {
            char data[1024];
            int size;
            getDataSegmentWithGottenSize(ntohl(rsp->size), data, &size);

            MSG_UPGRADE_DATA_REQ *req = (MSG_UPGRADE_DATA_REQ *)alloc_simcomUpgradeDataReq(ntohl(rsp->size), data, size);
            if (!req)
            {
                LOG_FATAL("insufficient memory");
            }

            simcom_sendMsg(req, sizeof(MSG_UPGRADE_DATA_REQ) + size, session);
        }
        else
        {
            LOG_INFO("send upgrade end request");

            int checksum = getLastFileChecksum();
            
            MSG_UPGRADE_END_REQ *req4end = (MSG_UPGRADE_END_REQ *)alloc_simcomUpgradeEndReq(checksum, LastSize);
            if (!req4end)
            {
                LOG_FATAL("insufficient memory");
            }

            simcom_sendMsg(req4end, sizeof(MSG_UPGRADE_END_REQ), session);
        }
    }
    else
    {
        LOG_INFO("response get upgrade data size(%d) error", ntohl(rsp->size));
    }
    
    return 0;
}

static int simcom_UpgradeEnd(const void *msg, SESSION *session)
{
    const MSG_UPGRADE_END_RSP *rsp = (const MSG_UPGRADE_END_RSP *)msg;

    if(rsp->code == 0)
    {
        LOG_INFO("get upgrade end rsponse, ok");
    }
    else
    {
        LOG_ERROR("get upgrade end rsponse, error code(%d)", rsp->code);
    }
    
    return 0;
}

static int simcom_SimInfo(const void *msg, SESSION *session)
{
    //get CMD_SIM_INFO req
    //remember CCID and IMSI
    
    return 0;
}

static int simcom_DeviceInfoGet(const void *msg, SESSION *session)
{
    const MSG_HEADER *req = (const MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("req handle empty");
        return -1;
    }
    if(ntohs(req->length) < sizeof(CGI) + 6 && ntohs(req->length) < sizeof(GPS) + 6)
    {
        LOG_ERROR("device info message length not enough");
        return -1;
    }

    OBJECT * obj = (OBJECT *) session->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    //parse for gps or cell
    const char *isGPS = (const char *)(req + 1);
    const char *autolock;
    if(*isGPS == 0x01)
    {
        //gps
        const GPS *gps = (const GPS *)(isGPS + 1);
        if (!gps)
        {
            LOG_ERROR("gps handle empty");
            return -1;
        }

        LOG_INFO("Device Info GPS: timestamp(%d), latitude(%f), longitude(%f), speed(%d), course(%d)",
            ntohl(gps->timestamp), gps->latitude, gps->longitude, gps->speed, ntohs(gps->course));

        obj->timestamp = ntohl(gps->timestamp);
        obj->isGPSlocated = 0x01;
        obj->lat = gps->latitude;
        obj->lon = gps->longitude;
        obj->speed = gps->speed;
        obj->course = ntohs(gps->course);

        autolock = (const char *)(gps + 1);
    }
    else if(*isGPS == 0x00)
    {
        //one cell
        const short *mcc = (const short *)(isGPS + 1);
        if (!mcc)
        {
            LOG_ERROR("mcc handle empty");
            return -1;
        }

        LOG_INFO("Device Info Cell: mcc(%d), mnc(%d), lac(%d), cid(%d)", ntohs(*mcc), ntohs(*(mcc+1)), ntohs(*(mcc+2)), ntohs(*(mcc+3)));

        obj->isGPSlocated = 0x00;
        /* unused
        obj->mcc = ntohs(*mcc);
        obj->mnc = ntohs(*(mcc+1));
        obj->lac = ntohs(*(mcc+2));
        obj->cid = ntohs(*(mcc+4));
        */

        autolock = (const char *)(mcc + 4);
    }

    //parse for autolock, autoperiod, percent, miles, status
    LOG_INFO("Device Info Others: autolock(%d), autoperiod(%d), percent(%d), miles(%d), status(%d)",
            *autolock, *(autolock+1), *(autolock+2), *(autolock+3), *(autolock+4));

    app_sendStatusGetRsp2App(APP_CMD_STATUS_GET, CODE_SUCCESS, obj, 
        *autolock, *(autolock+1), *(autolock+2), *(autolock+3), *(autolock+4));

    return 0;
}

static int simcom_gpsPack(const void *msg, SESSION *session)
{
    const MSG_HEADER *req = (const MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(ntohs(req->length) < sizeof(GPS))
    {
        LOG_ERROR("gps message length not enough");
        return -1;
    }

    const GPS *gps = (const GPS *)(req + 1);
    if (!gps)
    {
        LOG_ERROR("gps handle empty");
        return -1;
    }

    OBJECT * obj = (OBJECT *)session->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    int num = ntohs(req->length) / sizeof(GPS);
    for(int i = 0; i < num; ++i)
    {
        obj->timestamp = ntohl(gps[i].timestamp);
        obj->lat = gps[i].latitude;
        obj->lon = gps[i].longitude;
        obj->speed = gps[i].speed;
        obj->course = ntohs(gps[i].course);

        LOG_INFO("GPS_PACK(%d/%d): timestamp(%d), latitude(%f), longitude(%f), speed(%d), course(%d)",
                i+1, num, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);

        db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);
        sync_gps(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);
    }
    obj->isGPSlocated = 0x01;

    //send the last gps in GPS_PACK to app
    app_sendGpsMsg2App(session);

    return 0;
}

static MSG_PROC_MAP msgProcs[] =
{
    {CMD_WILD,              simcom_wild},
    {CMD_LOGIN,             simcom_login},
    {CMD_PING,              simcom_ping},
    {CMD_GPS,               simcom_gps},
    {CMD_CELL,              simcom_cell},
    {CMD_ALARM,             simcom_alarm},
    {CMD_SMS,               simcom_sms},
    {CMD_433,               simcom_433},
    {CMD_DEFEND,            simcom_defend},
    {CMD_SEEK,              simcom_seek},
    {CMD_LOCATE,            simcom_locate},
    {CMD_SET_TIMER,         simcom_SetTimer},
    {CMD_SET_AUTOSWITCH,    simcom_SetAutoswitch},
    {CMD_GET_AUTOSWITCH,    simcom_GetAutoswitch},
    {CMD_SET_PERIOD,        simcom_SetPeriod},
    {CMD_GET_PERIOD,        simcom_GetPeriod},
    {CMD_ITINERARY,         simcom_itinerary},
    {CMD_BATTERY,           simcom_battery},
    {CMD_DEFEND_ON,         simcom_DefendOn},
    {CMD_DEFEND_OFF,        simcom_DefendOff},
    {CMD_DEFEND_GET,        simcom_DefendGet},
    {CMD_DEFEND_NOTIFY,     simcom_DefendNotify},
    {CMD_UPGRADE_START,     simcom_UpgradeStart},
    {CMD_UPGRADE_DATA,      simcom_UpgradeData},
    {CMD_UPGRADE_END,       simcom_UpgradeEnd},
    {CMD_SIM_INFO,          simcom_SimInfo},
    {CMD_DEVICE_INFO_GET,   simcom_DeviceInfoGet},
    {CMD_GPS_PACK,          simcom_gpsPack}
};

int handle_one_msg(const void *m, SESSION *ctx)
{
    const MSG_HEADER *msg = (const MSG_HEADER *)m;

    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg, ctx);
            }
        }
    }

    return -1;
}

int handle_simcom_msg(const char *m, size_t msgLen, void *arg)
{
    const MSG_HEADER *msg = (const MSG_HEADER *)m;

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }
    size_t leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN)
    {
        const unsigned char *status = (const unsigned char *)(&(msg->signature));
        if((status[0] != 0xaa) || (status[1] != 0x55))
        {
            LOG_ERROR("receive message header signature error:%x", (unsigned)ntohs(msg->signature));
            return -1;
        }
        handle_one_msg(msg, (SESSION *)arg);
        leftLen = leftLen - MSG_HEADER_LEN - ntohs(msg->length);
        msg = (const MSG_HEADER *)(m + msgLen - leftLen);
    }
    return 0;
}
