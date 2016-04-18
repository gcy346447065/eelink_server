/*
 * msg_manager.c
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */

#include <malloc.h>
#include <netinet/in.h>

#include "msg_manager.h"
#include "protocol_manager.h"

static char seq = 0;

MSG_HEADER *alloc_manager_msg(char cmd, size_t length)
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

MSG_HEADER *alloc_manager_rspMsg(const MSG_HEADER *pMsg)
{
    size_t msgLen = 0;
    switch (pMsg->cmd)
    {
        case CMD_LOGIN:
            msgLen = sizeof(MSG_LOGIN_RSP);
            break;

        case CMD_IMEI_DATA:
            msgLen = sizeof(MSG_IMEI_DATA_RSP);
            break;

        default:
            return NULL;
    }

    MSG_HEADER *msg = malloc(msgLen);
    if(msg)
    {
        msg->signature = htons(START_FLAG);
        msg->cmd = pMsg->cmd;
        msg->seq = pMsg->seq;
        msg->length = htons(msgLen - MSG_HEADER_LEN);
    }

    return msg;
}

void free_manager_msg(void *msg)
{
    free(msg);
}

char get_manager_msg_cmd(void *msg)
{
    return ((MSG_HEADER *)msg)->cmd;
}

