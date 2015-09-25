/*
 * msg_proc_simcom.c
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */


#include <string.h>
#include <netinet/in.h>
#include <malloc.h>

#include "msg_proc_app.h"
#include "msg_proc_simcom.h"
#include "protocol.h"
#include "log.h"
#include "session.h"
#include "object.h"
#include "msg_simcom.h"
#include "db.h"
#include "mqtt.h"
#include "cJSON.h"
#include "yunba_push.h"
#include "msg_app.h"

typedef int (*MSG_PROC)(const void *msg, SESSION *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

int handle_simcom_msg(const char *m, size_t msgLen, void *arg);
int simcom_msg_send(void *msg, size_t len, SESSION *ctx);

static int handle_one_msg(const void *msg, SESSION *ctx);
static int get_msg_cmd(const void *msg);

static int simcom_login(const void *msg, SESSION *ctx);
static int simcom_gps(const void *msg, SESSION *ctx);
static int simcom_cell(const void *msg, SESSION *ctx);
static int simcom_ping(const void *msg, SESSION *ctx);
static int simcom_alarm(const void *msg, SESSION *ctx);
static int simcom_433(const void *msg, SESSION *ctx);
static int simcom_defend(const void *msg, SESSION *ctx);
static int simcom_seek(const void *msg, SESSION *ctx);


static MSG_PROC_MAP msgProcs[] =
{
        {CMD_LOGIN,     simcom_login},
        {CMD_GPS,       simcom_gps},
        {CMD_CELL,      simcom_cell},
        {CMD_PING,      simcom_ping},
        {CMD_ALARM,     simcom_alarm},
        {CMD_433,       simcom_433},
        {CMD_DEFEND,    simcom_defend},
        {CMD_SEEK,      simcom_seek},
};

int handle_simcom_msg(const char *m, size_t msgLen, void *arg)
{
    const MSG_HEADER *msg = (MSG_HEADER *)m;

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }
    size_t leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN)
    {
        unsigned char *status = (unsigned char *)(&(msg->signature));
        if((status[0] != 0xaa) || (status[1] != 0x55))
        {
            LOG_ERROR("receive message header signature error:%x", (unsigned)ntohs(msg->signature));
            return -1;
        }
        handle_one_msg(msg, (SESSION *)arg);
        leftLen = leftLen - MSG_HEADER_LEN - ntohs(msg->length);
        msg = (MSG_HEADER *)(m + msgLen - leftLen);
    }
    return 0;
}

int simcom_msg_send(void *msg, size_t len, SESSION *ctx)
{
    if (!ctx)
    {
        return -1;
    }

    SIMCOM_MSG_SEND pfn = ctx->pSendMsg;
    if (!pfn)
    {
        LOG_ERROR("device offline");
        return -1;
    }

    pfn(ctx->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_msg_cmd(msg), len);
    LOG_HEX(msg, len);

    free(msg);

    return 0;
}

//--------------------------------Utility Function-------------------------

int handle_one_msg(const void *m, SESSION *ctx)
{
    int i = get_msg_cmd(m);
    if(i == -1)
    {
        LOG_ERROR("unknown message cmd(%d)", ((MSG_HEADER *)m)->cmd);
        return -1;
    }

    return (msgProcs[i].pfn)(m, ctx);
}

int get_msg_cmd(const void *m)
{
    const MSG_HEADER *msg = (MSG_HEADER *)m;

    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            return i;
        }
    }
    return -1;
}

//-----------------------------Handles for Msg--------------------------------

int simcom_login(const void *msg, SESSION *ctx)
{
    const MSG_LOGIN_REQ* req = msg;

    OBJECT * obj = ctx->obj;

    const char *imei = getIMEI(req->IMEI);

    if(!obj)
    {
        LOG_DEBUG("mc IMEI(%s) login", imei);

        obj = obj_get(imei);

        if (!obj)
        {
            LOG_INFO("the first time of simcom IMEI(%s)'s login", imei);

            obj = obj_new();

            memcpy(obj->IMEI, imei, IMEI_LENGTH + 1);
            memcpy(obj->DID, imei, strlen(req->IMEI) + 1);

            obj_add(obj);
            mqtt_subscribe(obj->IMEI);
        }

        ctx->obj = obj;
        session_add(ctx);
    }
    else
    {
        LOG_DEBUG("simcom IMEI(%s) already login", imei);
    }

    MSG_LOGIN_RSP *rsp = alloc_simcom_rspMsg(msg);
    if (rsp)
    {
        simcom_msg_send(rsp, sizeof(MSG_LOGIN_RSP), ctx);
    }
    else
    {
        free(rsp);
        LOG_ERROR("insufficient memory");
        return -1;
    }

    if(!db_isTableCreated(obj->IMEI))
    {
        db_createCGI(obj->IMEI);
        db_createGPS(obj->IMEI);
    }

    return 0;
}

int simcom_gps(const void *msg, SESSION *ctx)
{
    const MSG_GPS* req = msg;

    if (!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }

    if (req->header.length < sizeof(MSG_GPS) - MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    LOG_INFO("GPS: lat(%f), lng(%f)", req->gps.latitude, req->gps.longitude);

    OBJECT * obj = (OBJECT *)ctx->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }
    //no response message needed

    time_t rawtime;
    time(&rawtime);
    obj->timestamp = rawtime;

    if (obj->lat == req->gps.latitude
        && obj->lon == req->gps.longitude)
    {
        LOG_INFO("No need to save data to leancloud");
    }
    else
    {
        //update local object
        obj->isGPSlocated = 0x01;
        obj->lat = req->gps.latitude;
        obj->lon = req->gps.longitude;
        //leancloud_saveGPS(obj, ctx);
    }

    app_sendGpsMsg2App(ctx);
    //stop upload data to yeelink
    //yeelink_saveGPS(obj, ctx);

    db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, 0, 0);

    return 0;
}

int simcom_cell(const void *msg, SESSION *ctx)
{
    const MSG_HEADER *req = (MSG_HEADER *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->length < sizeof(CGI))
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    const CGI *cgi = (CGI *)(req + 1);

    LOG_INFO("CGI: mcc(%d), mnc(%d)", ntohs(cgi->mcc), ntohs(cgi->mnc));
    OBJECT *obj = ctx->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    time_t rawtime;
    time(&rawtime);
    obj->timestamp = rawtime;
    obj->isGPSlocated = 0x00;

    int num = cgi->cellNo;
    if(num > CELL_NUM)
    {
        LOG_ERROR("Number:%d of cell is over", num);
        return -1;
    }
    const CELL *cell = (CELL *)(cgi + 1);

    for(int i = 0; i < num; ++i)
    {
        (obj->cell[i]).mcc = ntohs(cgi->mcc);
        (obj->cell[i]).mnc = ntohs(cgi->mnc);
        (obj->cell[i]).lac = ntohs((cell[i]).lac);
        (obj->cell[i]).ci = ntohs((cell[i]).cellid);
        (obj->cell[i]).rxl = ntohs((cell[i]).rxl);
        db_saveCGI(obj->IMEI, obj->timestamp, (obj->cell[i]).mcc, (obj->cell[i]).mnc, (obj->cell[i]).lac, (obj->cell[i]).ci, (obj->cell[i]).rxl);
    }

    //TODO: how to tranform cgi to gps

    return 0;
}

int simcom_ping(const void *msg, SESSION *ctx)
{
    return 0;
}

int simcom_alarm(const void *msg, SESSION *ctx)
{
    const MSG_ALARM_REQ *req = (MSG_ALARM_REQ *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->header.length < sizeof(MSG_ALARM_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    LOG_INFO("ALARM: %d", req->alarmType);
    OBJECT *obj = ctx->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }
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

int simcom_433(const void *msg, SESSION *ctx)
{
    const MSG_433 *req = (MSG_433 *)msg;
    if(!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }
    if(req->header.length < sizeof(MSG_433) - MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    LOG_INFO("433: %d", req->intensity);
    OBJECT *obj = ctx->obj;
    if(!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }

    app_send433Msg2App(ntohl(req->intensity), ctx);
    return 0;
}

int simcom_defend(const void *msg, SESSION *ctx)
{
    //send ack to APP
    static short seq = 0;
    MSG_DEFEND_RSP *rsp = (MSG_DEFEND_RSP *)msg;
    unsigned char defend = ((object *)(ctx->obj))->defend;
    if(defend == CMD_FENCE_SET)
    {
        if(rsp->result == 0)
        {
            app_sendRspMsg2App(CMD_FENCE_SET, seq++, NULL, 0, ctx);
        }
    }
    else if(defend == CMD_FENCE_DEL)
    {
        if(rsp->result == 0)
        {
            app_sendRspMsg2App(CMD_FENCE_DEL, seq++, NULL, 0, ctx);
        }
    }
    else if(defend == CMD_FENCE_GET)
    {
        if(rsp->result == DEFEND_ON)
        {
            app_sendRspMsg2App(CMD_FENCE_SET, seq++, NULL, 0, ctx);
        }
        else
        {
            app_sendRspMsg2App(CMD_FENCE_DEL, seq++, NULL, 0, ctx);
        }
    }
    else
    {
        LOG_ERROR("response defend cmd error");
        return -1;
    }
    return 0;
}

int simcom_seek(const void *msg, SESSION *ctx)
{
    //send ack to APP
    static short seq = 0;
    MSG_SEEK_RSP *rsp = (MSG_SEEK_RSP *)msg;
    if(rsp->result == SEEK_ON)
    {
        app_sendRspMsg2App(CMD_SEEK_ON, seq++, NULL, 0, ctx);
    }
    else if(rsp->result == SEEK_OFF)
    {
        app_sendRspMsg2App(CMD_SEEK_OFF, seq++, NULL, 0, ctx);
    }
    else
    {
        LOG_ERROR("response seek cmd not exist");
        return -1;
    }
    return 0;
}
