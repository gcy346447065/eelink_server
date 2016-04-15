/*
 * msg_proc_manager.c
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */

#include "log.h"
#include "session.h"
#include "protocol.h" //added for MSG_HEADER and MSG_HEADER_LEN
#include "msg_proc_manager.h"

typedef int (*MSG_PROC)(const void *msg, SESSION *ctx);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
} MSG_PROC_MAP;

static int manager_ready(const void *msg, SESSION *ctx)
{
    LOG_INFO("%s", (const char *)msg);

    return 0;
}

static MSG_PROC_MAP msgProcs[] =
{
    {1,              manager_ready}
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
