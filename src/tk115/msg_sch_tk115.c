/*
 * schedule_mc.c
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <assert.h>

#include "msg_tk115.h"
#include "msg_sch_tk115.h"
#include "msg_proc_tk115.h"

#include "log.h"

#ifdef WITH_CATEGORY

#define LOG_DEBUG(...) \
	zlog(cat[MOD_SCH_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_SCH_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_SCH_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_SCH_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_SCH_MC], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
#endif

typedef int (*MSG_PROC)(const void* msg, SESSION* ctx);
typedef struct
{
	char cmd;
	MSG_PROC pfn;
} MSG_PROC_MAP;



static MSG_PROC_MAP msgProcs[] =
{
		{CMD_LOGIN, tk115_login},
		{CMD_GPS, tk115_gps},
		{CMD_PING, tk115_ping},
		{CMD_ALARM, tk115_alarm},
		{CMD_STATUS, tk115_status},
		{CMD_SMS, tk115_sms},
		{CMD_OPERAT, tk115_operator},
		{CMD_DATA, tk115_data},
};

int handle_one_msg(const void* m, SESSION* ctx)
{
	MC_MSG_HEADER* msg = (MC_MSG_HEADER*)m;


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

int handle_msg(const char *m, size_t msgLen, SESSION *ctx)
{
	MC_MSG_HEADER* msg = (MC_MSG_HEADER *)m;

	if (msgLen < sizeof(MC_MSG_HEADER))
	{
		LOG_ERROR("receive message length not enough: %zu(at least(%zu)", msgLen, sizeof(MC_MSG_HEADER));

		return -1;
	}

	size_t leftMsgLen = msgLen;
	while (leftMsgLen >= MC_MSG_HEADER_LEN + ntohs(msg->length))
	{
		//check the msg header
		if (msg->header[0] != 0x67 || msg->header[1] != 0x67)
		{
			LOG_ERROR("receive message header signature error: %x%x)", msg->header[0], msg->header[1]);

			return -1;
		}

		handle_one_msg(msg, ctx);

		leftMsgLen = leftMsgLen - MC_MSG_HEADER_LEN - ntohs(msg->length);
		msg = (MC_MSG_HEADER *)(m + (msgLen - leftMsgLen));
	};

	return 0;
}


