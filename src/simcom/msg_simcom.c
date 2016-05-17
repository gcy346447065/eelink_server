/*
 * msg_simcom.c
 *
 *  Created on: 2015/6/29
 *      Author: jk
 */

#include <malloc.h>
#include <netinet/in.h>
#include <string.h>
#include "msg_simcom.h"

static char seq = 0;

MSG_HEADER* alloc_simcom_msg(char cmd, size_t length)
{
    MSG_HEADER* msg = malloc(length);

    if(msg)
    {
        msg->signature = htons(START_FLAG);
        msg->cmd = cmd;
        msg->seq = seq++;
        msg->length = htons(length - MSG_HEADER_LEN);
    }

    return msg;
}

MSG_HEADER* alloc_simcom_rspMsg(const MSG_HEADER *pMsg)
{
    size_t msgLen = 0;
    switch (pMsg->cmd)
    {
        case CMD_LOGIN:
            msgLen = sizeof(MSG_LOGIN_RSP);
            break;

        case CMD_SMS:
            msgLen = sizeof(MSG_SMS_RSP);
            break;

        case CMD_ITINERARY:
            msgLen = sizeof(MSG_ITINERARY_RSP);
            break;

        case CMD_SIM_INFO:
            msgLen = sizeof(MSG_SIM_INFO_RSP);
            break;

        case CMD_ALARM:
            msgLen = sizeof(MSG_ALARM_RSP);
            break;

        default:
            return NULL;
    }

    MSG_HEADER* msg = malloc(msgLen);

    msg->signature = htons(START_FLAG);
    msg->cmd = pMsg->cmd;
    msg->seq = pMsg->seq;
    msg->length = htons(msgLen - MSG_HEADER_LEN);

    return msg;
}

void free_simcom_msg(void* msg)
{
    free(msg);
}

char get_msg_cmd(void *msg)
{
    return ((MSG_HEADER*)msg)->cmd;
}

void *alloc_simcomWildMsg(const char* data, size_t length)
{
    MSG_HEADER *msg = alloc_simcom_msg(CMD_WILD, length);

    if (msg)
    {
        void *msgBody = msg + 1;
        memcpy(msgBody, data, length);
    }

    return msg;
}

void *alloc_simcomDefendReq(int token, char operator)
{
    MSG_DEFEND_REQ *req = (MSG_DEFEND_REQ *)alloc_simcom_msg(CMD_DEFEND, sizeof(MSG_DEFEND_REQ));
    if(req)
    {
        req->token = htonl(token);
        req->operator = operator;
    }

    return req;
}

void *alloc_simcomSeekReq(int token, char operator)
{
    MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcom_msg(CMD_SEEK, sizeof(MSG_SEEK_REQ));
    if(req)
    {
        req->token = htonl(token);
        req->operator = operator;
    }

    return req;
}

void *alloc_simcomAutolockSetReq(int token, char onOff)
{
    MSG_AUTOLOCK_SET_REQ *req = (MSG_AUTOLOCK_SET_REQ *)alloc_simcom_msg(CMD_SET_AUTOSWITCH, sizeof(MSG_AUTOLOCK_SET_REQ));
    if(req)
    {
        req->token = htonl(token);
        req->onOff = onOff;
    }

    return req;
}

void *alloc_simcomAutoPeriodSetReq(int token, char period)
{
    MSG_AUTOPERIOD_SET_REQ *req = (MSG_AUTOPERIOD_SET_REQ *)alloc_simcom_msg(CMD_SET_PERIOD, sizeof(MSG_AUTOPERIOD_SET_REQ));
    if(req)
    {
        req->token = htonl(token);
        req->period = period;
    }

    return req;
}

void *alloc_simcomAutoPeriodGetReq(int token)
{
    MSG_AUTOPERIOD_GET_REQ *req = (MSG_AUTOPERIOD_GET_REQ *)alloc_simcom_msg(CMD_GET_PERIOD, sizeof(MSG_AUTOPERIOD_GET_REQ));
    if(req)
    {
        req->token = htonl(token);
    }

    return req;
}

void *alloc_simcomAutolockGetReq(int token)
{
    MSG_AUTOLOCK_GET_REQ *req = (MSG_AUTOLOCK_GET_REQ *)alloc_simcom_msg(CMD_GET_AUTOSWITCH, sizeof(MSG_AUTOLOCK_GET_REQ));
    if(req)
    {
        req->token = htonl(token);
    }

    return req;
}

void *alloc_simcomUpgradeStartReq(int version, int size)
{
    MSG_UPGRADE_START_REQ *req = (MSG_UPGRADE_START_REQ *)alloc_simcom_msg(CMD_UPGRADE_START, sizeof(MSG_UPGRADE_START_REQ));
    if(req)
    {
        req->version = htonl(version);
        req->size = htonl(size);
    }

    return req;
}

void *alloc_simcomUpgradeDataReq(int offset, char *data, int length)
{
    MSG_UPGRADE_DATA_REQ *req = (MSG_UPGRADE_DATA_REQ *)alloc_simcom_msg(CMD_UPGRADE_DATA, sizeof(MSG_UPGRADE_DATA_REQ) + length);
    if(req)
    {
        req->offset = htonl(offset);
        memcpy(req->data, data, length);
    }

    return req;
}

void *alloc_simcomUpgradeEndReq(int checksum, int size)
{
    MSG_UPGRADE_END_REQ *req = (MSG_UPGRADE_END_REQ *)alloc_simcom_msg(CMD_UPGRADE_END, sizeof(MSG_UPGRADE_END_REQ));
    if(req)
    {
        req->checksum = htonl(checksum);
        req->size = htonl(size);
    }

    return req;
}

void *alloc_simcomManagerReq(int cmd)
{
    size_t msgLen = 0;
    switch (cmd)
    {
        case CMD_GET_LOG:
            msgLen = sizeof(MSG_GET_LOG_REQ);
            break;

        case CMD_GET_433:
            msgLen = sizeof(MSG_GET_433_REQ);
            break;

        case CMD_GET_GSM:
            msgLen = sizeof(MSG_GET_GSM_REQ);
            break;

        case CMD_GET_GPS:
            msgLen = sizeof(MSG_GET_GPS_REQ);
            break;

        case CMD_GET_SETTING:
            msgLen = sizeof(MSG_GET_SETTING_REQ);
            break;

        case CMD_GET_BATTERY:
            msgLen = sizeof(MSG_GET_BATTERY_REQ);
            break;

        case CMD_REBOOT:
            msgLen = sizeof(MSG_REBOOT_REQ);
            break;

        case CMD_UPGRADE:
            msgLen = sizeof(MSG_UPGRADE_REQ);
            break;

        default:
            return NULL;
    }

    MSG_HEADER* msg = malloc(msgLen);

    msg->signature = htons(START_FLAG);
    msg->cmd = cmd;
    msg->seq = seq++;
    msg->length = htons(msgLen - MSG_HEADER_LEN);

    return msg;
}

