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
#include "log.h"
#include "protocol.h"

static unsigned short seq = 0;

MSG_HEADER* alloc_simcom_msg(char cmd, size_t length)
{
    MSG_HEADER* msg = malloc(length);

    if (msg)
    {
        msg->signature = htons(START_FLAG);
        msg->cmd = cmd;
        msg->seq = htons(seq++);
        msg->length = htons(length - MSG_HEADER_LEN);
    }

    return msg;
}

MSG_HEADER* alloc_simcom_rspMsg(const MSG_HEADER* pMsg)
{
    size_t msgLen = 0;
    switch (pMsg->cmd)
    {
    case CMD_LOGIN:
        msgLen = sizeof(MSG_LOGIN_RSP);
        break;

    case CMD_PING:
        msgLen = sizeof(MSG_PING_RSP);
        break;

    case CMD_SMS:
        msgLen = sizeof(MSG_SMS_RSP);    //FIXME: without any sms contents
        break;

    default:
        return NULL;
    }

    MSG_HEADER* msg = malloc(msgLen);

    msg->signature = htons(START_FLAG);
    msg->cmd = pMsg->cmd;
    msg->length = htons(msgLen - MSG_HEADER_LEN);
    msg->seq = htons(pMsg->seq);

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

const char *getIMEI(const char *imei)
{
    static char ret[IMEI_LENGTH + 1];
    memcpy(ret, imei, IMEI_LENGTH);
    ret[IMEI_LENGTH] = '\0';
    return ret;
}


void* alloc_simcomDefendReq(int token, unsigned char operator)
{
    MSG_DEFEND_REQ *req = (MSG_DEFEND_REQ *)alloc_simcom_msg(CMD_DEFEND, sizeof(MSG_DEFEND_REQ));
    if(!req)
    {
        req->token = token;
        req->operator = operator;
    }

    return req;
}
