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

typedef int (*MSG_PROC)(const void *msg, SESSION *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int handle_one_msg(const void *msg, SESSION *ctx);
static int simcom_login(const void *msg, SESSION *ctx);
static int simcom_gps(const void *msg, SESSION *ctx);
static int simcom_cell(const void *msg, SESSION *ctx);
static int simcom_ping(const void *msg, SESSION *ctx);
static int simcom_alarm(const void *msg, SESSION *ctx);
static int get_msg_cmd(const void *msg);

static MSG_PROC_MAP msgProcs[] =
{
        {CMD_LOGIN, simcom_login},
        {CMD_GPS,   simcom_gps},
        {CMD_CELL,  simcom_cell},
        {CMD_PING,  simcom_ping},
        {CMD_ALARM, simcom_alarm},
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


int handle_one_msg(const void *m, SESSION *ctx)
{
    const MSG_HEADER *msg = (MSG_HEADER *)m;
    for (size_t i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg, ctx);
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


static int simcom_msg_send(void *msg, size_t len, SESSION *ctx)
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

static int simcom_login(const void *msg, SESSION *ctx)
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
        //TODO: LOG_ERROR
    }

    if(!db_isTableCreated(obj->IMEI))
    {
        db_createCGI(obj->IMEI);
        db_createGPS(obj->IMEI);
    }

    return 0;
}

static int simcom_gps(const void *msg, SESSION *ctx)
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

    if (obj->lat == ntohl(req->gps.latitude)
        && obj->lon == ntohl(req->gps.longitude))
    {
        LOG_INFO("No need to save data to leancloud");
    }
    else
    {
        //update local object
        obj->isGPSlocated = 0x01;
        obj->lat = ntohl(req->gps.latitude);
        obj->lon = ntohl(req->gps.longitude);
        //leancloud_saveGPS(obj, ctx);
    }

    app_sendGpsMsg2App(ctx);
    //stop upload data to yeelink
    //yeelink_saveGPS(obj, ctx);

    db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, 0, 0);

    return 0;
}

static int simcom_cell(const void *msg, SESSION *ctx)
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

    CGI *cgi = req + 1;

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

    for(int i = 0; i < num; ++i)
    {
        (obj->cell[i]).mcc = ntohs(cgi->mcc);
        (obj->cell[i]).mnc = ntohs(cgi->mnc);
        (obj->cell[i]).lac = ntohs((cgi->cell[i]).lac);
        (obj->cell[i]).ci = ntohs((cgi->cell[i]).cellid);
        (obj->cell[i]).rxl = ntohs((cgi->cell[i]).rxl);
        db_saveCGI(obj->IMEI, obj->timestamp, (obj->cell[i]).mcc, (obj->cell[i]).mnc, (obj->cell[i]).lac, (obj->cell[i]).ci, (obj->cell[i]).rxl);
    }

    //TODO: how to tranform cgi to gps

    return 0;
}

static int simcom_ping(const void *msg, SESSION *ctx)
{
    return 0;
}

static int simcom_alarm(const void *msg, SESSION *ctx)
{
    return 0;
}

static int get_msg_cmd(const void *m)
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