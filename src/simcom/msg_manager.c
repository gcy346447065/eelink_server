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

MANAGER_MSG_HEADER *alloc_manager_msg(char cmd, size_t length)
{
    MANAGER_MSG_HEADER* msg = malloc(length);

    if(msg)
    {
        msg->signature = htons(MANAGER_START_FLAG);
        msg->cmd = cmd;
        msg->seq = seq++;
        msg->length = htons(length - MANAGER_MSG_HEADER_LEN);
    }

    return msg;
}

MANAGER_MSG_HEADER *alloc_manager_rspMsg(const MANAGER_MSG_HEADER *pMsg)
{
    size_t msgLen = 0;
    switch (pMsg->cmd)
    {
        case MANAGER_CMD_LOGIN:
            msgLen = sizeof(MANAGER_MSG_LOGIN_RSP);
            break;

        case MANAGER_CMD_IMEI_DATA:
            msgLen = sizeof(MANAGER_MSG_IMEI_DATA_RSP);
            break;

        default:
            return NULL;
    }

    MANAGER_MSG_HEADER *msg = malloc(msgLen);
    if(msg)
    {
        msg->signature = htons(MANAGER_START_FLAG);
        msg->cmd = pMsg->cmd;
        msg->seq = pMsg->seq;
        msg->length = htons(msgLen - MANAGER_MSG_HEADER_LEN);
    }

    return msg;
}

void free_manager_msg(void *msg)
{
    free(msg);
}

char get_manager_msg_cmd(void *msg)
{
    return ((MANAGER_MSG_HEADER *)msg)->cmd;
}
