/*
 * msg_proc_mc.c
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */
#include <assert.h>
#include <string.h>
#include <time.h>

#include "msg_proc_tk115.h"
#include "msg_tk115.h"
#include "object.h"
#include "session.h"
#include "msg_proc_app.h"
#include "cJSON.h"
#include "yunba_push.h"
#include "log.h"
#include "mqtt.h"
#include "db.h"


int msg_send(void *msg, size_t len, SESSION *ctx)
{
    if (!ctx)
    {
        return -1;
    }
    
    MSG_SEND pfn = ctx->pSendMsg;
    if (!pfn)
    {
            LOG_ERROR("device offline");
            return -1;
    }

    pfn(ctx->bev, msg, len);

    LOG_DEBUG("send msg(cmd=%d), length(%ld)", get_msg_cmd(msg), len);
    LOG_HEX(msg, len);

    return 0;
}

int tk115_login(const void *msg, SESSION *ctx)
{
	const MC_MSG_LOGIN_REQ* req = msg;


	OBJECT * obj = ctx->obj;
	if (!obj)
	{
		const char* strIMEI = get_IMEI_STRING(req->IMEI);
		LOG_DEBUG("mc IMEI(%s) login", strIMEI);

		obj = obj_get(strIMEI);

		if (!obj)
		{
			LOG_INFO("the first time of object(%s) login: language(%s), locale(%d)",
					strIMEI,
					req->language ? "EN" : "CN",
					req->locale / 4);

			obj = obj_new();

			memcpy(obj->IMEI, strIMEI, IMEI_LENGTH + 1);
			memcpy(obj->DID, strIMEI, strlen(strIMEI));
			obj->language = req->language;
			obj->locale = req->locale;

//			leancloud_saveDid(obj);

			//add object to table and db
			obj_add(obj);
			mqtt_subscribe(obj->IMEI);
		}

		ctx->obj = obj;
	}
	else
	{
		LOG_DEBUG("mc IMEI(%s) already login", get_IMEI_STRING(req->IMEI));
	}

    session_add(ctx);


	MC_MSG_LOGIN_RSP *rsp = (MC_MSG_LOGIN_RSP *)alloc_rspMsg(msg);
	if (rsp)
	{
		msg_send(rsp, sizeof(MC_MSG_LOGIN_RSP), ctx);
	}
	else
	{
		//TODO: LOG_ERROR
	}

	if (!db_isTableCreated(obj->IMEI))
    {
        db_createCGI(obj->IMEI);
        db_createGPS(obj->IMEI);
    }

	return 0;
}

int tk115_gps(const void *msg, SESSION *ctx)
{
	const MC_MSG_GPS_REQ* req = msg;

	if (!req)
	{
		LOG_ERROR("msg handle empty");
		return -1;
	}

	if (req->header.length < sizeof(MC_MSG_LOGIN_REQ) - MC_MSG_HEADER_LEN)
	{
		LOG_ERROR("message length not enough");
		return -1;
	}

    //transform gps from int to double
    //static const int transGPS = 1800000;
    //double t_lat = (double)ntohl(req->lat) / transGPS;
    //double t_lon = (double)ntohl(req->lon) / transGPS;

	LOG_INFO("GPS: lat(%f), lng(%f), speed(%d), course(%d), GPS(%s)",
			t_lat,
			t_lon,
			req->speed,
			ntohs(req->course),
			req->location & 0x01 ? "YES" : "NO");

	OBJECT * obj = (OBJECT *)ctx->obj;
	if (!obj)
	{
		LOG_WARN("MC must first login");
		return -1;
	}

	if(!(req->location&0x01) && obj->lon != 0)
	{
		LOG_INFO("no gps,only send to app");
		time_t rawtime;
		time ( &rawtime );
		obj->timestamp = rawtime;
		obj->isGPSlocated = 0;
		app_sendGpsMsg2App(ctx);
		return 0;
	}
	
	//problem about double equal
	if (obj->lat == t_lat
		&& obj->lon == t_lon
		&& obj->speed == req->speed
		&& obj->course == ntohs(req->course))
	{
		LOG_INFO("No need to save data to leancloud");
		obj->timestamp = ntohl(req->timestamp);
		obj->isGPSlocated = req->location & 0x01;
		app_sendGpsMsg2App(ctx);
		return 0;
	}

	//update local object
	obj->lat = t_lat;
	obj->lon = t_lon;
	obj->speed = req->speed;
	obj->course = ntohs(req->course);
	obj->timestamp = ntohl(req->timestamp);
	obj->isGPSlocated = req->location & 0x01;

	(obj->cell[0]).mcc = (req->cell).mcc;
	(obj->cell[0]).mnc = (req->cell).mnc;
	(obj->cell[0]).lac = (req->cell).lac;
	//transform ci from char[3] to short
	(obj->cell[0]).ci = *(short *)(&((req->cell).ci[1]));

	app_sendGpsMsg2App(ctx);

	//stop upload data to yeelink
	//yeelink_saveGPS(obj, ctx);

    //TODO:save GPS to database
	if (req->location & 0x01)
    {
        //int db_saveGPS(const char *name, int timestamp, int lat, int lon, char speed, short course)
        db_saveGPS(obj->IMEI, obj->timestamp, obj->lat, obj->lon, obj->speed, obj->course);
    }
    else
    {
        //int db_saveCGI(const char *name, int timestamp, short mcc, short mnc, short lac, short ci, short rxl)
        db_saveCGI(obj->IMEI, obj->timestamp, (obj->cell[0]).mcc, (obj->cell[0]).mnc, (obj->cell[0]).lac, (obj->cell[0]).ci, 0);
    }


//	leancloud_saveGPS(obj);

	return 0;
}

int tk115_ping(const void *msg, SESSION *ctx)
{
	const MC_MSG_PING_REQ *req = msg;

	short status = ntohs(req->status);

	LOG_INFO("GPS located:%s", (status & 1) ? "YES" : "NO");

	MC_MSG_PING_RSP* rsp = (MC_MSG_PING_RSP *)alloc_rspMsg(msg);
	if (rsp)
	{
		msg_send(rsp, sizeof(MC_MSG_PING_RSP), ctx);
	}

	return 0;
}

int tk115_alarm(const void *msg, SESSION *ctx)
{
	OBJECT * obj = ctx->obj;

	if (!obj)
	{
		LOG_WARN("MC does not login");
		return -1;
	}

	const MC_MSG_ALARM_REQ* req = msg;

	switch (req->type)
	{
	case FENCE_IN:
	{
		LOG_INFO("FENCE_IN Alarm");
		break;
	}
	case FENCE_OUT:
		LOG_INFO("FENCE_OUT Alarm");
		break;

	default:
		LOG_INFO("Alarm type = %#x", req->type);
	}


	obj->lat = ntohl(req->lat);
	obj->lon = ntohl(req->lon);
	obj->speed = req->speed;
	obj->course = ntohs(req->course);

	(obj->cell[0]).mcc = (req->cell).mcc;
	(obj->cell[0]).mnc = (req->cell).mnc;
	(obj->cell[0]).lac = (req->cell).lac;
	(obj->cell[0]).ci = *(short *)(&((req->cell).ci[1]));

	MC_MSG_ALARM_RSP* rsp = NULL;
	size_t rspMsgLength = 0;
	if(req->location & 0x01)
	{
		char alarm_message[]={0xE7,0x94,0xB5,0xE5,0x8A,0xA8,0xE8,0xBD,0xA6,0xe7,0xa7,0xbb,0xe5,0x8a,0xa8,0xe6,0x8a,0xa5,0xe8,0xad,0xa6};
		rspMsgLength = sizeof(MC_MSG_ALARM_RSP) + sizeof(alarm_message);
		rsp = (MC_MSG_ALARM_RSP *)alloc_msg(req->header.cmd, rspMsgLength);
		if (rsp)
		{
			memcpy(rsp->sms,alarm_message,sizeof(alarm_message));
		}
	}
	else
	{
		rspMsgLength = sizeof(MC_MSG_ALARM_RSP);
		rsp = (MC_MSG_ALARM_RSP *)alloc_msg(req->header.cmd, rspMsgLength);
	}

	if (rsp)
	{
		set_msg_seq(&rsp->header, get_msg_seq((const MC_MSG_HEADER *)req));
		msg_send(&rsp->header, rspMsgLength, ctx);
	}
	else
	{
		LOG_FATAL("no memory");
	}

	if (!(req->location & 0x01))
	{
		LOG_WARN("GPS not located, don't send alarm");
		return 0;
	}


	//send the alarm to YUNBA
	char topic[128];
	memset(topic, 0, sizeof(topic));
	snprintf(topic, 128, "e2link_%s", obj->IMEI);

	cJSON *root = cJSON_CreateObject();

	cJSON *alarm = cJSON_CreateObject();
	cJSON_AddNumberToObject(alarm,"type", req->type);

	cJSON_AddItemToObject(root, "alarm", alarm);

	char* json = cJSON_PrintUnformatted(root);

	yunba_publish(topic, json, strlen(json));
	LOG_INFO("send alarm: %s", topic);

	free(json);
	cJSON_Delete(root);

	return 0;
}

int tk115_status(const void *msg, SESSION *ctx)
{
	const MC_MSG_STATUS_REQ* req = msg;

	OBJECT * obj = ctx->obj;
	if (!obj)
	{
		LOG_WARN("MC must first login");
		return -1;
	}

	switch (req->type)
	{
	case ACC_ON:
		LOG_INFO("STATUS: acc on");
		break;
	case ACC_OFF:
		LOG_INFO("STATUS: acc off");
		break;
	case DIGTAL:
		LOG_INFO("STATUS: digital port status changed");
		break;
	default:
		break;
	}

	MC_MSG_STATUS_RSP* rsp = (MC_MSG_STATUS_RSP *)alloc_rspMsg(msg);
	if (rsp)
	{
		msg_send(rsp, sizeof(MC_MSG_PING_RSP), ctx);
	}

	return 0;
}

int tk115_sms(const void *msg, SESSION *ctx)
{
	const MC_MSG_SMS_REQ* req = msg;

	LOG_INFO("GPS located:%s", (req->location & 1) ? "YES" : "NO");

	MC_MSG_SMS_RSP* rsp = (MC_MSG_SMS_RSP *)alloc_rspMsg(msg);
	if (rsp)
	{
		msg_send(rsp, sizeof(MC_MSG_PING_RSP), ctx);
	}
	return 0;
}

int tk115_operator(const void *msg, SESSION *ctx)
{
	OBJECT * obj = ctx->obj;

	const MC_MSG_OPERATOR_RSP* req = msg;

	switch (req->type)
	{
	case 0x01:
	{
		int session = req->token;

		short cmd = (session & 0xffff0000) >> 16;
		short seq = session & 0xffff;
		app_sendRspMsg2App(cmd, seq, (void *)req->data, sizeof(MC_MSG_HEADER) + ntohs(req->header.length) - sizeof(MC_MSG_OPERATOR_RSP), ctx);
		break;
	}

	case 0x02:
		//TODO:handle the msg
		break;

	default:
		break;
	}

	LOG_INFO("MC operator response: %s", req->data);

	return 0; //TODO:
}

int tk115_data(const void *msg, SESSION *ctx __attribute__((unused)))
{
	LOG_INFO("MC data message");

	return 0;
}

