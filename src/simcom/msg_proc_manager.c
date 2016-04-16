/*
 * msg_proc_manager.c
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */
#include <string.h>

#include "log.h"
#include "object.h" //for send imei data
#include "session.h"
#include "msg_manager.h"
#include "protocol_manager.h"
#include "msg_proc_manager.h"

typedef int (*MSG_PROC)(const void *msg, SESSION *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int manager_sendMsg(void *msg, size_t len, SESSION *session)
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

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_manager_msg_cmd(msg), len);
    LOG_HEX(msg, len);
    free_manager_msg(msg);

    return 0;
}

int manager_sendImeiData(const void *msg, SESSION *ManagerSession, const char *imei, SESSION *deviceSession, int timestamp, float lon, float lat, char speed, short course)
{
    char online_offline;
    if(deviceSession == NULL)
    {
        online_offline = 2; //offline
    }
    else
    {
        online_offline = 1; //online
    }

    MSG_IMEI_DATA_RSP *rsp = (MSG_IMEI_DATA_RSP *)alloc_manager_rspMsg((const MSG_HEADER *)msg);
    if(rsp)
    {
        memcpy(rsp->imei_data.IMEI, imei, MAX_IMEI_LENGTH);
        rsp->imei_data.online_offline = online_offline;
        rsp->imei_data.gps.timestamp = timestamp;
        rsp->imei_data.gps.longitude = lon;
        rsp->imei_data.gps.latitude = lat;
        rsp->imei_data.gps.speed = speed;
        rsp->imei_data.gps.course = course;

        manager_sendMsg(rsp, sizeof(MSG_IMEI_DATA_RSP), ManagerSession);
        LOG_INFO("send imei data rsp");
    }
    else
    {
        free_manager_msg(rsp);
        LOG_ERROR("insufficient memory");
        return -1;
    }

    return 0;
}

static int manager_login(const void *msg, SESSION *session)
{
    const MSG_LOGIN_REQ *req = (const MSG_LOGIN_REQ *)msg;
    if(ntohs(req->length) != sizeof(MSG_LOGIN_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("login message length not enough");
        return -1;
    }

    //login rsp
    MSG_LOGIN_RSP *rsp = (MSG_LOGIN_RSP *)alloc_manager_rspMsg((const MSG_HEADER *)msg);
    if(rsp)
    {
        manager_sendMsg(rsp, sizeof(MSG_LOGIN_RSP), session);
        LOG_INFO("send login rsp");
    }
    else
    {
        free_manager_msg(rsp);
        LOG_ERROR("insufficient memory");
        return -1;
    }

    return 0;
}

static int manager_imeiData(const void *msg, SESSION *session)
{
    const MSG_IMEI_DATA_REQ *req = (const MSG_IMEI_DATA_REQ *)msg;
    if(ntohs(req->length) != sizeof(MSG_IMEI_DATA_REQ) - MSG_HEADER_LEN)
    {
        LOG_ERROR("imei data message length not enough");
        return -1;
    }

    LOG_INFO("get imei data req");

    //loop for sending imei data rsp
    obj_sendImeiData2ManagerLoop((const void *)msg, session, manager_sendImeiData);

    return 0;
}

static MSG_PROC_MAP msgProcs[] =
{
    {CMD_LOGIN,             manager_login},
    {CMD_IMEI_DATA,         manager_imeiData}
};

static int handle_one_msg(const void *m, SESSION *ctx)
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

int handle_manager_msg(const char *m, size_t msgLen, void *arg)
{
    const MSG_HEADER *msg = (const MSG_HEADER *)m;

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, MSG_HEADER_LEN);

        return -1;
    }
    size_t leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN)
    {
        const unsigned char *status = (const unsigned char *)(&(msg->signature));
        if((status[0] != 0xaa) || (status[1] != 0x66))
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
