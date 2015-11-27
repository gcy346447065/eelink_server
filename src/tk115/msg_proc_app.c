/*
 * msg_proc_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <object.h>

#include "msg_app.h"
#include "msg_tk115.h"
#include "msg_proc_tk115.h"
#include "msg_proc_app.h"
#include "log.h"
#include "macro.h"
#include "session.h"
#include "object.h"
#include "mqtt.h"


static void app_sendRawData2TK115(const void* msg, int len, const char *imei, int token)
{
	OBJECT* obj = obj_get(imei);
	SESSION *session = obj->session;
	if(!session)
	{
		LOG_ERROR("obj %s offline", imei);
        return;
	}

	MC_MSG_OPERATOR_REQ* req = (MC_MSG_OPERATOR_REQ *)alloc_msg(CMD_OPERAT, sizeof(MC_MSG_OPERATOR_REQ) + len);
	if(req)
	{
		req->type = 0x01;	//FIXME: use enum instead
		req->token = token;
		memcpy(req->data, msg, len);
        msg_send(req, sizeof(MC_MSG_OPERATOR_REQ) + len, session);
	}
	else
	{
		LOG_FATAL("insufficient memory");
	}
}

static void app_sendRawData2App(const char* topic, char *data, int len)
{
	LOG_HEX(data, len);

	mqtt_publish(topic, data, len);
	free(data);
}

void app_sendRspMsg2App(short cmd, short seq, void* data, int len, void* session)
{
	if (!session)
	{
		LOG_FATAL("internal error: session null");
		return;
	}

	OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
	if (!obj)
	{
		LOG_FATAL("internal error: obj null");
		return;
	}

	APP_MSG* msg = malloc(sizeof(APP_MSG) + len);
	if (!msg)
	{
		LOG_FATAL("insufficient memory");
		return;
	}

	msg->header = htons(0xAA55);
	msg->cmd = htons(cmd);
	msg->length = htons(len + sizeof(msg->seq));
	msg->seq = htons(seq);
	memcpy(msg->data, data, len);

	char topic[IMEI_LENGTH + 20] = {0};

	snprintf(topic, IMEI_LENGTH + 20, "dev2app/%s/e2link/cmd", obj->IMEI);

	app_sendRawData2App(topic, (char *)msg, sizeof(APP_MSG) + len);
}



void app_sendGpsMsg2App(void* session)
{
	OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
	if (!obj)
	{
		LOG_ERROR("obj null, no data to upload");
		return;
	}

	GPS_MSG* msg = malloc(sizeof(GPS_MSG));
	if (!msg)
	{
		LOG_FATAL("insufficient memory");
		return;
	}

	msg->header = htons(0xAA55);
	msg->timestamp = htonl(obj->timestamp);
	msg->lat = htonl(obj->lat);
	msg->lon = htonl(obj->lon);
	msg->course = htons(obj->course);
	msg->speed = obj->speed;
	msg->isGPS = obj->isGPSlocated;

	char topic[IMEI_LENGTH + 20] = {0};
	snprintf(topic, IMEI_LENGTH + 20, "dev2app/%s/e2link/gps", obj->IMEI);

	app_sendRawData2App(topic, (char *)msg, sizeof(GPS_MSG));
}

int app_handleApp2devMsg(const char* topic, const char* data, const int len)
{
	LOG_HEX(data, len);

	//check the IMEI
	const char* pStart = &topic[strlen("app2dev/")];
	const char* pEnd = strstr(pStart, "/");
	char strIMEI[IMEI_LENGTH + 1] = {0};
	if (pEnd - pStart != IMEI_LENGTH)
	{
		LOG_ERROR("app2dev: imei length has a problem");
		return -1;
	}

	strncpy(strIMEI, pStart, IMEI_LENGTH);

	// OBJECT* obj = obj_get(strIMEI);
	// if (!obj)
	// {
	// 	LOG_ERROR("obj %s not exist", strIMEI);
	// 	return -1;
	// }

	APP_MSG* pMsg = (APP_MSG *)data;
	if (!pMsg)
	{
		LOG_FATAL("internal error: msg null");
		return -1;
	}

	//check the msg header
	if (ntohs(pMsg->header) != 0xAA55)
	{
		LOG_ERROR("App2dev msg header error");
		return -1;
	}

	//check the msg length
	if ((ntohs(pMsg->length) + sizeof(short) * 3) != len)
	{
		LOG_ERROR("App2dev msg length error");
		return -1;
	}

	short cmd = ntohs(pMsg->cmd);
	short seq = ntohs(pMsg->seq);
	int token = (cmd << 16) + seq;

	LOG_INFO("receive app CMD:%#x", cmd);
	app_sendRawData2TK115(pMsg->data, ntohs(pMsg->length) - sizeof(pMsg->seq), strIMEI, token);

	return 0;
}

