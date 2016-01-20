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
#include "macro.h"

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

int simcom_wild(const void *m, SESSION *session)
{
	const MSG_HEADER *msg = m;
    const char *msg_log = msg + 1;

    LOG_DEBUG("DBG:%s", msg_log);
	app_sendDebugMsg2App(msg_log, msg->length, session);

	return 0;
}

int simcom_login(const void *msg, SESSION *session)
{
    const MSG_LOGIN_REQ *req = (const MSG_LOGIN_REQ *)msg;
    const char *imei = getImeiStringFromArray(req->IMEI);

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

            obj = obj_new();

            memcpy(obj->IMEI, imei, IMEI_LENGTH + 1);
            memcpy(obj->DID,  imei, IMEI_LENGTH + 1);//IMEI and DID mean the same now
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
        LOG_DEBUG("simcom IMEI(%s) already login", imei);
    }

    MSG_LOGIN_RSP *rsp = alloc_simcom_rspMsg(msg);
    if (rsp)
    {
        simcom_sendMsg(rsp, sizeof(MSG_LOGIN_RSP), session);
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


    return 0;
}

int simcom_ping(const void *msg, SESSION *session)
{
    msg = msg;
    session = session;

    LOG_INFO("get simcom ping");

    return 0;
}

int simcom_gps(const void *msg, SESSION *session)
{
    const MSG_GPS *req = (const MSG_GPS *)msg;
    if (!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if (req->header.length < sizeof(MSG_GPS) - MSG_HEADER_LEN)
    {
        LOG_ERROR("gps message length not enough");
        return -1;
    }

    LOG_INFO("GPS: latitude(%f), longitude(%f), altitude(%f), speed(%f), course(%f)", 
        req->gps.latitude, req->gps.longitude, req->gps.altitude, req->gps.speed, req->gps.course);

    OBJECT * obj = (OBJECT *) session->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    obj->timestamp = get_time();

    if (fabs(obj->lat - req->gps.latitude) < FLT_EPSILON
        && fabs(obj->lon - req->gps.longitude) < FLT_EPSILON)
    {
        LOG_INFO("No need to save data to leancloud");
    }
    else
    {
        //update local object
        obj->isGPSlocated = 0x01;
        obj->lat = req->gps.latitude;
        obj->lon = req->gps.longitude;
    }

    obj->altitude = req->gps.altitude;
    obj->speed = (char)req->gps.speed;
    obj->course = (short)req->gps.course;

    app_sendGpsMsg2App(session);

    //TO DO: add altitude to db
    db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);

    sync_gps(obj->IMEI, obj->lat, obj->lon, obj->altitude, obj->speed, obj->course);

    return 0;
}

int simcom_cell(const void *msg, SESSION *session)
{
    const MSG_HEADER *req = (const MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->length < sizeof(CGI))
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

    int num = cgi->cellNo;
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

int simcom_alarm(const void *msg, SESSION *session)
{
    const MSG_ALARM_REQ *req = (const MSG_ALARM_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->header.length < sizeof(MSG_ALARM_REQ) - MSG_HEADER_LEN)
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
    cJSON_AddNumberToObject(alarm,"type", req->alarmType);
    cJSON_AddItemToObject(root, "alarm", alarm);
    char* json = cJSON_PrintUnformatted(root);

    yunba_publish(topic, json, strlen(json));
    LOG_INFO("send alarm: %s", topic);

    free(json);
    cJSON_Delete(root);

    return 0;
}

int simcom_sms(const void *msg , SESSION *session )
{
    msg = msg;
    session = session;

    return 0;
}

int simcom_433(const void *msg, SESSION *session)
{
    const MSG_433 *req = (const MSG_433 *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->header.length < sizeof(MSG_433) - MSG_HEADER_LEN)
    {
        LOG_ERROR("433 message length not enough");
        return -1;
    }

    LOG_INFO("433: %d", req->intensity);
    OBJECT *obj = session->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    app_send433Msg2App(get_time(), ntohl(req->intensity), session);
    return 0;
}

int simcom_defend(const void *msg, SESSION *session)
{
    //send ack to APP
    const MSG_DEFEND_RSP *rsp = (const MSG_DEFEND_RSP *)msg;
    int defend = rsp->token;

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
            app_sendCmdRsp2App(APP_CMD_FENCE_ON, ERR_SUCCESS, strIMEI);
        }
    }
    else if(defend == APP_CMD_FENCE_OFF)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_FENCE_OFF, ERR_SUCCESS, strIMEI);
        }
    }
    else if(defend == APP_CMD_FENCE_GET)
    {
        if(rsp->result == DEFEND_ON)
        {
            app_sendFenceGetRspMsg2App(APP_CMD_FENCE_GET, ERR_SUCCESS, 1, session);
        }
        else
        {
            app_sendFenceGetRspMsg2App(APP_CMD_FENCE_GET, ERR_SUCCESS, 0, session);
        }
    }
    else
    {
        LOG_ERROR("response defend cmd error");
        return -1;
    }
    return 0;
}

int simcom_seek(const void *msg, SESSION *session)
{
    //send ack to APP
    const MSG_SEEK_RSP *rsp = (const MSG_SEEK_RSP *)msg;
    int seek = rsp->token;

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
            app_sendCmdRsp2App(APP_CMD_SEEK_ON, ERR_SUCCESS, strIMEI);
        }
    }
    else if(seek == APP_CMD_SEEK_OFF)
    {
        if(rsp->result == 0)
        {
            app_sendCmdRsp2App(APP_CMD_SEEK_OFF, ERR_SUCCESS, strIMEI);
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
    if(req->length < sizeof(CGI) && req->length < sizeof(GPS))
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

        LOG_INFO("GPS: latitude(%f), longitude(%f), altitude(%f), speed(%u), course(%d)", 
            gps->latitude, gps->longitude, gps->altitude, gps->speed, gps->course);

        OBJECT * obj = (OBJECT *) session->obj;
        if (!obj)
        {
            LOG_WARN("MC must first login");
            return -1;
        }

        obj->timestamp = get_time();
        obj->isGPSlocated = 0x01;
        obj->lat = gps->latitude;
        obj->lon = gps->longitude;
        obj->altitude = gps->altitude;
        obj->speed = (char)gps->speed;
        obj->course = (short)gps->course;

        app_sendGpsMsg2App(session);
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

        LOG_INFO("CGI: mcc(%d), mnc(%d)", ntohs(cgi->mcc), ntohs(cgi->mnc));
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
        obj->altitude = 0;
        obj->speed = 0;
        obj->course = 0;

        app_sendGpsMsg2App(session);
    }

    return 0;
}

static int simcom_SetServer(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_SetTimer(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_SetAutoswitch(const void *msg, SESSION *session)
{
    const MSG_AUTODEFEND_SWITCH_SET_RSP *rsp = (const MSG_AUTODEFEND_SWITCH_SET_RSP*)msg;
    OBJECT* obj = session->obj;

    app_sendCmdRsp2App(APP_CMD_AUTOLOCK_ON, rsp->result, obj->IMEI);
    return 0;
}

static int simcom_GetAutoswitch(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_SetPeriod(const void *msg, SESSION *session)
{
    const MSG_AUTODEFEND_PERIOD_SET_RSP *rsp = (const MSG_AUTODEFEND_PERIOD_SET_RSP*)msg;
    OBJECT *obj = session->obj;

    app_sendCmdRsp2App(APP_CMD_AUTOPERIOD_SET, rsp->result, obj->IMEI);
    return 0;
}

static int simcom_GetPeriod(const void *msg, SESSION *session)
{
    //TODO: to be complted?
    const MSG_AUTODEFEND_PERIOD_GET_RSP* rsp = (const MSG_AUTODEFEND_PERIOD_GET_RSP *)msg;
    OBJECT *obj = session->obj;

    app_sendAutoDefendPeriodMsg2App(get_time(), rsp->period, session);

    return 0;
}

static int simcom_itinerary(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_battery(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_DefendOn(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_DefendOff(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_DefendGet(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

    return 0;
}

static int simcom_DefendNotify(const void *msg, SESSION *session)
{
    //TODO: to be complted
    msg = msg;
    session = session;

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
        {CMD_SET_SERVER,        simcom_SetServer},
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
        {CMD_DEFEND_NOTIFY,     simcom_DefendNotify}
    };

int handle_one_msg(const void *m, SESSION *ctx)
{
    const MSG_HEADER* msg = (MSG_HEADER*)m;

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
