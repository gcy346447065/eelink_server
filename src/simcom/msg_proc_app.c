/*
 * msg_proc_app.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <netinet/in.h>

#include "msg_app.h"
#include "msg_simcom.h"
#include "msg_proc_simcom.h"
#include "log.h"
#include "macro.h"
#include "session.h"
#include "object.h"
#include "mqtt.h"
#include "protocol.h"
#include "cJSON.h"

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata);

void app_sendGpsMsg2App(void *session);
void app_send433Msg2App(int timestamp, int intensity, void * session);
void app_sendCmdMsg2App(char *cmd, int result, char *state, void *session);

static void app_sendRawData2mc(const char *imei, const void *msg, size_t len);
static void app_sendRawData2App(const char *topic, const void *msg, size_t len);

 //----------------------------send raw msg to app/mc------------------------------------------
static void app_sendRawData2mc(const char *imei, const void* msg, size_t len)
{
	SESSION *session = session_get(imei);
	if(!session)
	{
		LOG_ERROR("obj %s offline", imei);
        return;
	}
	int rc = simcom_msg_send(msg, len, session);
	if(rc)
	{
		LOG_ERROR("send msg to simcom error");
	}
}

static void app_sendRawData2App(const char* topic, const void *msg, size_t len)
{
	LOG_HEX(data, len);
	mqtt_publish(topic, msg, len);
}


//----------------------------send cmd/gps/433 to app-----------------------------------------
void app_sendCmdMsg2App(char *cmd, int result, char *state, void* session)
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

	char topic[IMEI_LENGTH + 13];
	memset(topic, 0, sizeof(topic));
	snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/cmd", obj->IMEI);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", cmd);
	cJSON_AddNumberToObject(root, "result", result);
	cJSON_AddStringToObject(root, "state", state);

	char *json = cJSON_PrintUnformatted(root);

	app_sendRawData2App(topic, json, strlen(json));
	LOG_INFO(send cmd msg to APP);
	free(json);
	cJSON_Delete(root);
}

void app_sendGpsMsg2App(void* session)
{
	OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
	if (!obj)
	{
		LOG_ERROR("obj null, no data to upload");
		return;
	}
	char topic[IMEI_LENGTH + 13];
	memset(topic, 0, sizeof(topic));
	snprintf(topic, IMEI_LENGTH + 20, "dev2app/%s/gps", obj->IMEI);

	cJSON * root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "timestamp", obj->timestamp);
	cJSON_AddNumberToObject(root, "lat", obj->lat);
	cJSON_AddNumberToObject(root, "lng", obj->lon);

	char *json = cJSON_PrintUnformatted(root);

	app_sendRawData2App(topic, json, strlen(json));
	LOG_INFO("send gps msg to APP");
	free(json);
	cJSON_Delete(root);
}

void app_send433Msg2App(int timestamp, int intensity, void * session)
{
	OBJECT* obj = (OBJECT *)((SESSION *)session)->obj;
	if (!obj)
	{
		LOG_ERROR("obj null, no data to upload");
		return;
	}
	char topic[IMEI_LENGTH + 13];
	memset(topic, 0, sizeof(topic));
	snprintf(topic, IMEI_LENGTH + 13, "dev2app/%s/433", obj->IMEI);

	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "timestamp", timestamp);
	cJSON_AddNumberToObject(root, "intensity", intensity);

	char *json = cJSON_PrintUnformatted(root);

	app_sendRawData2App(topic, json, strlen(json));
	LOG_INFO("send 433 msg to APP");
	free(json);
	cJSON_Delete(root);
}


//---------------------------------handle for msg from app------------------------------------
static char defendApp2mc(int cmd)
{
	if(cmd == CMD_FENCE_SET)
	{
		return DEFEND_ON;
	}
	else if(cmd == CMD_FENCE_DEL)
	{
		return DEFEND_OFF;
	}
	else
	{
		return DEFEND_GET;
	}
}

static char seekApp2mc(int cmd)
{
	if(cmd == CMD_SEEK_ON)
	{
		return SEEK_ON;
	}
	else
	{
		return SEEK_OFF;
	}
}

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata)
{
	APP_MSG *pMsg = (APP_MSG *)data;
	if (!pMsg)
	{
		LOG_FATAL("internal error: msg null");
		return -1;
	}

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

	OBJECT* obj = obj_get(strIMEI);
	if (!obj)
	{
		LOG_ERROR("obj %s not exist", strIMEI);
		return -1;
	}
	SESSION *ctx = session_get(strIMEI);
	if(!ctx)
	{
		LOG_ERROR("simcom %s offline", strIMEI);
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
//	int token = (cmd << 16) + seq;

	switch (cmd)
	{
	case CMD_WILD:
		LOG_INFO("receive app wildcard cmd");
		//app_sendRawData2mc(pMsg->data, ntohs(pMsg->length) - sizeof(pMsg->seq), ctx, token);
		break;
	case CMD_FENCE_SET:
	case CMD_FENCE_DEL:
	case CMD_FENCE_GET:
	{
		LOG_INFO("receive app CMD_FENCE_%d", cmd);
		obj->defend = cmd;
		MSG_DEFEND_REQ *req = (MSG_DEFEND_REQ *)alloc_simcom_msg(CMD_DEFEND, sizeof(MSG_DEFEND_REQ));
		if(!req)
		{
			LOG_FATAL("insufficient memory");
			return -1;
		}
		req->operator = defendApp2mc(cmd);
		app_sendRawData2mc(req, sizeof(MSG_DEFEND_REQ), strIMEI);
		break;
	}
	case CMD_SEEK_ON:
	case CMD_SEEK_OFF:
		LOG_INFO("receive app CMD_SEEK_MODE cmd");
		obj->seek = cmd;
		MSG_SEEK_REQ *req = (MSG_SEEK_REQ *)alloc_simcom_msg(CMD_SEEK, sizeof(MSG_SEEK_REQ));
		if(!req)
		{
			LOG_FATAL("insufficient memory");
			return -1;
		}
		req->operator = seekApp2mc(cmd);
		app_sendRawData2mc(req, sizeof(MSG_SEEK_REQ), strIMEI);
		break;
	case CMD_GPS_GET:
		LOG_INFO("receive app CMD_GPS_GET");
		app_sendGpsMsg2App(ctx);
		break;
	default:
		LOG_ERROR("Unknown cmd: %#x", ntohs(pMsg->cmd));
		break;
	}
	return 0;
}

