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
#include "cb_ctx_simcom.h"
#include "object.h"
#include "msg_simcom.h"

typedef int (*MSG_PROC)(const void* msg, SIMCOM_CTX* ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int simcom_login(const void* msg, SIMCOM_CTX* ctx);
static int simcom_gps(const void* msg, SIMCOM_CTX* ctx);
static int simcom_cell(const void* msg, SIMCOM_CTX* ctx);
static int simcom_ping(const void* msg, SIMCOM_CTX* ctx);
static int simcom_alarm(const void* msg, SIMCOM_CTX* ctx);


static MSG_PROC_MAP msgProcs[] =
{
        {CMD_LOGIN, simcom_login},
        {CMD_GPS,   simcom_gps},
		{CMD_CELL,	simcom_cell},
        {CMD_PING,  simcom_ping},
        {CMD_ALARM, simcom_alarm},
};


int handle_simcom_msg(const char* m, size_t msgLen, void* arg)
{
    MSG_HEADER* msg = (MSG_HEADER*)m;

    if (msgLen < sizeof(MSG_HEADER))
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }

    if (msg->signature != ntohs(START_FLAG))
    {
        LOG_ERROR("message head signature error:%d", msg->signature);
        return -1;
    }

    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg, arg);
            }
            else
            {
                LOG_ERROR("Message %d not processed", msg->cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown message cmd(%d)", msg->cmd);
    return -1;
}


static int simcom_msg_send(void* msg, size_t len, SIMCOM_CTX* ctx)
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

static int simcom_login(const void* msg, SIMCOM_CTX* ctx)
{
    const MSG_LOGIN_REQ* req = msg;


    OBJECT * obj = ctx->obj;
    if (!obj)
    {
        LOG_DEBUG("mc IMEI(%s) login", req->IMEI);

        obj = obj_get(req->IMEI);

        if (!obj)
        {
            LOG_INFO("the first time of simcom IMEI(%s)'s login", req->IMEI);

            obj = obj_new();

            memcpy(obj->IMEI, req->IMEI, IMEI_LENGTH);
            memcpy(obj->DID, req->IMEI, strlen(req->IMEI));

            obj_add(obj);
        }

        ctx->obj = obj;
    }
    else
    {
        LOG_DEBUG("simcom IMEI(%s) already login", get_IMEI_STRING(req->IMEI));
    }


    MSG_LOGIN_RSP *rsp = alloc_simcom_rspMsg(msg);
    if (rsp)
    {
        simcom_msg_send(rsp, sizeof(MSG_LOGIN_RSP), ctx);
    }
    else
    {
        //TODO: LOG_ERROR
    }

    return 0;
}

static int simcom_gps(const void* msg, SIMCOM_CTX* ctx)
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

    OBJECT * obj = ctx->obj;
    if (!obj)
    {
        LOG_WARN("MC must first login");
        return -1;
    }
    //no response message needed


    if(obj->lon != 0)   //TODO
    {
        LOG_INFO("no gps,only send to app");
        time_t rawtime;
        time ( &rawtime );
        obj->timestamp = rawtime;
        obj->isGPSlocated = 0;
        app_sendGpsMsg2App(ctx);
        return 0;
    }

    if (obj->lat == ntohl(req->gps.latitude)
        && obj->lon == ntohl(req->gps.longitude))
    {
        LOG_INFO("No need to save data to leancloud");
        app_sendGpsMsg2App(ctx);
        return 0;
    }

    //update local object
    obj->lat = ntohl(req->gps.latitude);
    obj->lon = ntohl(req->gps.longitude);

    app_sendGpsMsg2App(ctx);

    //stop upload data to yeelink
    //yeelink_saveGPS(obj, ctx);

    return 0;
}

static int simcom_cell(const void* msg, SIMCOM_CTX* ctx)
{
    const MSG_HEADER* req = msg;

    if (!req)
    {
        LOG_ERROR("msg handle empty");
        return -1;
    }

    if (req->length < sizeof(CGI))
    {
        LOG_ERROR("message length not enough");
        return -1;
    }

    CGI* cgi = req + 1;

    LOG_INFO("CELL: mcc(%d), mnc(%d), total cell number = %d", cgi->mcc, cgi->mnc, cgi->cellNo);

    //TODO
    return 0;
}

static int simcom_ping(const void* msg, SIMCOM_CTX* ctx)
{
    return 0;
}

static int simcom_alarm(const void* msg, SIMCOM_CTX* ctx)
{
    return 0;
}
