/*
 * gizwits_rsp.c
 *
 *  Created on: Apr 21, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fsm.h"
#include "log.h"
#include "gizwits_rsp.h"
#include "cb_ctx_mc.h"
#include "macro_mc.h"
#include "object_mc.h"
#include "msg_gizwits.h"
#include "leancloud_req.h"

#ifdef WITH_CATEGORY

#define LOG_DEBUG(...) \
	zlog(cat[MOD_GIZWITS_RSP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_GIZWITS_RSP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_GIZWITS_RSP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_GIZWITS_RSP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_GIZWITS_RSP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)

#endif

int mc_register_rsp(int response_code, const char* msg, CB_CTX* ctx)
{
	LOG_DEBUG("process register response msg");

	if (response_code == 201)
	{
		//TODO: process the msg
		const char* p_didStart = msg + strlen("did=");
		OBJ_MC* obj = ctx->obj;
		memcpy(obj->DID, p_didStart, DID_LEN);
		LOG_INFO("Got did: %s", obj->DID);

		mc_saveConfig();
		leancloud_saveDid(obj, ctx);

		fsm_run(EVT_GOT_DID, ctx);
		return 0;
	}

	return -1;
}

int mc_provision_rsp(int response_code, const char* msg, CB_CTX* ctx)
{
	LOG_DEBUG("provision response: status_code = %d, msg = %s", response_code, msg);

	if (response_code == 200)
	{
		//TODO: process the msg: parse the m2m server's domain and port
	    char m2m_host[100] = {0};
	    char temp_port[10]={0};
	    int m2m_port;
	    memset(m2m_host, 0, 100);

	    char* p_start = strstr(msg, "host=");
	    if (!p_start)
	    {
	    	LOG_ERROR("can not get host start");
	    	return -1;
	    }
	    p_start += strlen("host=");
	    char* p_end = strstr(p_start, "&");
	    if (!p_end)
	    {
	    	LOG_ERROR("can not get host end");
	    	return -1;
	    }

	    memcpy(m2m_host, p_start, p_end - p_start);


	    p_start = strstr((p_end + 1), "port=");
	    if(!p_start)
	    {
	    	LOG_ERROR("can not get port start");
	    	return 1;
	    }
	    p_start += strlen("port=");

	    p_end = strstr(p_start,"&");
	    if (!p_end)
	    {
	    	LOG_ERROR("can not get port end");
	    }
	    memcpy(temp_port, p_start, p_end - p_start);
	    m2m_port = atoi(temp_port);

		OBJ_MC* obj = ctx->obj;

		memcpy(obj->m2m_host, m2m_host, strlen(m2m_host));
		obj->m2m_Port = m2m_port;

		fsm_run(EVT_GOT_M2M, ctx);
		return 0;
	}

	return -1;
}

static char mqtt_num_rem_len_bytes(const char* buf) {
	char num_bytes = 1;

	if ((buf[1] & 0x80) == 0x80) {
		num_bytes++;
		if ((buf[2] & 0x80) == 0x80) {
			num_bytes ++;
			if ((buf[3] & 0x80) == 0x80) {
				num_bytes ++;
			}
		}
	}
	return num_bytes;
}

static unsigned short mqtt_parse_rem_len(const char* buf) {
	unsigned short multiplier = 1;
	unsigned short value = 0;
	char digit;

	do {
		digit = *buf;
		value += (digit & 127) * multiplier;
		multiplier *= 128;
		buf++;
	} while ((digit & 128) != 0);

	return value;
}

int mqtt_app2dev(const char* topic, const char* data, const int len, void* userdata)
{
	OBJ_MC* obj = userdata;

    LOG_HEX(data, len);

	APP_SESSION* session = malloc(sizeof(APP_SESSION));

	const char* pStart = &topic[strlen("app2dev/")];
	const char* pEnd = strstr(pStart, "/");

	memcpy(session->DID, pStart, pEnd - pStart);

	//TODO: HOW to handle the did

	pStart = pEnd;
    strcpy(session->clientID, pStart + 1);

    /*
     * data format:
     * 		header: 4b  0x00000003
     * 		varlen: 1-4b (the following data len)
     * 		flag:	1b
     * 		cmd:	2b
     * 		data:	max 65536
     */

    int header = *(int*)data;
    if (ntohl(header) != 0x00000003)
    {
    	LOG_ERROR("app2dev msg header error: %x", header);
        return -1;
    }

    const char* pvarlen = data + sizeof(header);
    int varlen = mqtt_num_rem_len_bytes(pvarlen);
    int datalen = mqtt_parse_rem_len(pvarlen);

    typedef struct
    {
    	char flag;
    	short cmd;
    }__attribute__((__packed__)) CMD;
    CMD* cmd = pvarlen + varlen;
    LOG_DEBUG("app2dev msg: flag=%#x, cmd=%#x", cmd->flag, ntohs(cmd->cmd));

    const char* pDataToMc = cmd + 1;
    const int lenToMc = datalen - sizeof(CMD); // flag(1B)+cmd(2B)=3B

	//向设备发送控制指令
    #define		SUB_CMD_CONTROL_MCU			0x01
    //向设备请求设备状态
    #define		SUB_CMD_REQUIRE_STATUS		0x02
    //设备返回请求的设备状态
    #define		SUB_CMD_REQUIRE_STATUS_ACK	0x03
    //设备推送当前的设备状态
    #define		SUB_CMD_REPORT_MCU_STATUS	0x04

    typedef struct
    {
    	char sub_cmd; //
    	char cmd_tag;	// corresponding to the action in the MCU interface
    	//TODO: add writable data
    	char checksum;
    }__attribute__((__packed__)) REQ;
    REQ* req = pDataToMc;
    switch (req->sub_cmd)
	{
    case SUB_CMD_CONTROL_MCU:
    {
    	//根据相应的位，决定要修改哪个字段
    	if (req->cmd_tag & 0x01)
    	{
//		   char shift_msg[] = {1,1,2,3,4,'S','H','I','F','T',',','1','0','0','#'};
//		   char noshift_msg[] = {1,1,2,3,4,'S','H','I','F','T',',','0','#'};
//		   CB_CTX* ctx = container_of(obj, CB_CTX, obj);
//		   if (req->on_off)
//		   {
//			   send_raw_data2mc(shift_msg, sizeof(shift_msg), ctx, session);
//		   }
//		   else
//		   {
//			   send_raw_data2mc(noshift_msg, sizeof(noshift_msg), ctx, session);
//		   }

    	}

		if (req->cmd_tag & 0x02)
		{
			;
		}

    	break;
    }
    case SUB_CMD_REQUIRE_STATUS:
    {
    	GIZWITS_DATA giz;
    	giz.sub_cmd = SUB_CMD_REQUIRE_STATUS_ACK;
    	giz.lat = htonl((obj->lat / 30000.0 + 5400.0) * 10000);
    	giz.lon = htonl((obj->lon / 30000.0 + 10800.0) * 10000);
    	giz.speed = obj->speed;
    	giz.course = htons(obj->course);
    	//TODO:set all the other fields
    	send_data_giz(&giz, sizeof(giz), obj);
    	break;
    }
    default:
    	;
	}

    //send_raw_data2mc(pDataToMc, datalen, ctx, session);

	return 0;
}

int mqtt_ser2cli_res(const char* topic, const char* data, const int len, void* userdata)
{
	LOG_HEX(data, len);

	typedef struct
	{
		int header;
		short cmd;
		short appCount;
	}SER2CLI_RES;
	SER2CLI_RES* notify = data;

	if (notify)
	{
		LOG_INFO("Current online app is %d", ntohs(notify->appCount));
	}

	return 0;
}

int mqtt_ser2cli_noti(const char* topic, const char* data, const int len, void* userdata)
{
	LOG_HEX(data, len);


	return 0;
}

