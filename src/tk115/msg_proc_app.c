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

#include "msg_app.h"
#include "msg_tk115.h"
#include "msg_proc_tk115.h"
#include "msg_proc_app.h"
#include "log.h"
#include "macro.h"
#include "session.h"
#include "object.h"
#include "mqtt.h"


static void app_sendRawData2TK115(const void* msg, int len, const char* imei, int token)
{
	SESSION *session = session_get(imei);
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

static void app_sendRawData2App(const char* topic, char *data, int len, void* session)
{
	if (!session)
	{
		LOG_FATAL("internal error: session null");
		return;
	}

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

	app_sendRawData2App(topic, (char *)msg, sizeof(APP_MSG) + len, session);
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

	app_sendRawData2App(topic, (char *)msg, sizeof(GPS_MSG), session);
}

int app_handleApp2devMsg(const char* topic, const char* data, const int len, void* userdata)
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

//	OBJECT* obj = obj_get(strIMEI);
//	if (!obj)
//	{
//		LOG_ERROR("obj %s not exist", strIMEI);
//		return -1;
//	}

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

void app_message_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		LOG_DEBUG("%s %p", message->topic, message->payload);
	}else{
		LOG_DEBUG("%s no payload(null)", message->topic);
	}

	LOG_INFO("recieve PUBLISH: %s", message->topic);

	if(strncmp(message->topic,"app2dev/",strlen("app2dev/")) == 0)
	{
		app_handleApp2devMsg(message->topic, message->payload, message->payloadlen, userdata);
	}
	else
	{
		LOG_ERROR("Receive unknown PUBLISH");
	}

}

void app_connect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	if(!rc)
	{
		LOG_INFO("Connect to MQTT server successfully");
	}
	else
	{
		//TODO: check whether mosquitto_connack_string here is OK
		LOG_ERROR("Connect failed: result = %s", mosquitto_connack_string(rc));
	}
}

void app_disconnect_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata, int rc)
{

	if(rc)
	{
		LOG_ERROR("disconnect rc = %d(%s)\n",  rc, mosquitto_strerror(rc));

		//mosquitto_reconnect(mosq);
	}
	else
	{
		LOG_INFO("client disconnect successfully");
	}
}


void app_subscribe_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int mid, int qos_count, const int *granted_qos)
{
	LOG_DEBUG("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(int i=1; i<qos_count; i++){
		LOG_DEBUG(", %d", granted_qos[i]);
	}
}

void app_log_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int level, const char *str)
{
	switch (level)
	{
	case MOSQ_LOG_DEBUG:
		LOG_DEBUG("%s", str);

		break;
	case MOSQ_LOG_INFO:
	case MOSQ_LOG_NOTICE:
	case MOSQ_LOG_SUBSCRIBE:
	case MOSQ_LOG_UNSUBSCRIBE:
		LOG_INFO("%s", str);
		break;

	case MOSQ_LOG_WARNING:
		LOG_WARN("%s", str);
		break;

	case MOSQ_LOG_ERR:
		LOG_ERROR("%s", str);
		break;

	default:
		LOG_ERROR("unknown level log:%s", str);
	}

}

void app_publish_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int mid __attribute__((unused)))
{
	LOG_DEBUG("Published mid: %d", mid);
	LOG_INFO("Publish mid: %d successfully", mid);
}
