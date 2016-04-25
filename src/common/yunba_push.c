/*
 * yunba_push.c
 *
 *  Created on: May 14, 2015
 *      Author: jk
 */
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "yunba.h"
#include "yunba_push.h"
#include "log.h"

MQTTClient client;
MQTTClient_connectOptions conn_opts = { {'M', 'Q', 'T', 'C'}, 4, 60, 1, 1, NULL, NULL, NULL, 30, 20, NULL, 0, NULL, 0, {0}};

static int extendedCmdArrive(void *context __attribute__((unused)), EXTED_CMD cmd, int status, int ret_string_len, char *ret_string)
{
	char buf[1024];
	memset(buf, 0, 1024);
	memcpy(buf, ret_string, ret_string_len);
	LOG_INFO("extended cmd arrived:cmd=%02x,status=%02x,msg=%s\n", cmd, status, buf);

	return 0;
}

static int messageArrived(void* context __attribute__((unused)), char* topicName, int topicLen, MQTTClient_message* m)
{
	char action[30];
	char alias[60];
	int ret = -1;

	Presence_msg my_present;

	my_present.action = action;
	my_present.alias = alias;
	ret = get_present_info(topicName, m, &my_present);
	if (ret == 0)
	{
		LOG_INFO("action:%s alias:%s\n", my_present.action, my_present.alias);
	}

	time_t t;
	time(&t);
	LOG_DEBUG("Message arrived, date:%s", ctime(&t));
	LOG_DEBUG("     qos: %i", m->qos);
	LOG_DEBUG("     messageid: %"PRIu64"", m->msgid);
	LOG_DEBUG("     topic(%d): %s",topicLen, topicName);
	LOG_DEBUG("   message: ");

	LOG_HEX(m->payload, m->payloadlen);

	MQTTClient_freeMessage(&m);
	MQTTClient_free(topicName);

	/* not expecting any messages */
	return 1;
}

static void connectionLost(void *context __attribute__((unused)), char *cause)
{
	//The reason for the disconnection. Currently, cause is always set to NULL.
	LOG_WARN("yunba connect lost:%s", cause);
        int rc = MQTTClient_connect(client, &conn_opts);

	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("failed to re-connect:%d", rc);
		//FIXME: how to do when connect failed when lost connection
		//exit(-1);
	}
	else
	{
		LOG_INFO("re-connect ok");
	}

	return;
}

int yunba_connect()
{
	REG_info reg_info;

	int rc = MQTTClient_setup_with_appkey(YUNBA_APPKEY, &reg_info);
	if (rc < 0)
	{
		LOG_ERROR("can't setup with appkey: rc=%d", rc);
		return rc;
	}
	else
	{
		LOG_INFO("yunba reg info: client_id:%s, username:%s, password:%s, devide_id:%s\n",
				reg_info.client_id, reg_info.username, reg_info.password, reg_info.device_id);
	}

	char url[128] = {0};
	rc = MQTTClient_get_host(YUNBA_APPKEY, url);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("can't get host info: rc=%d", rc);
		return rc;
	}
	else
	{
		LOG_INFO("yunba url: %s", url);
	}

	rc = MQTTClient_create(&client, url, reg_info.client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("can't create client instance to yunba: rc=%d", rc);
		return rc;
	}
	else
	{
		LOG_INFO("create yunba client ok");
	}

	//TODO: set the callback
	rc = MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived, NULL, extendedCmdArrive);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("can't set yunba callback: rc=%d", rc);
		return rc;
	}

	conn_opts.keepAliveInterval = 300;
	conn_opts.username = reg_info.username;
	conn_opts.password = reg_info.password;
	rc = MQTTClient_connect(client, &conn_opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("can't connect to yunba:rc=%d", rc);
	    return rc;
	}

	//TODO: currently no topic subscribed
	//rc = MQTTClient_subscribe(client, "channel");

	return MQTTCLIENT_SUCCESS;
}

void yunba_disconnect()
{
    int rc = MQTTClient_disconnect(client, 10000);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("disconnect yunba error:rc = %d", rc);
	}

    MQTTClient_destroy(&client);
}

void yunba_publish_old(char* topicName, char* payload, int payloadLen)
{
//	int rc = MQTTClient_publish_json(client, topicName, data);
	//cJSON *apn_json, *aps;
	cJSON *Opt = cJSON_CreateObject();
	cJSON_AddStringToObject(Opt, "time_to_live", "120");
	cJSON_AddStringToObject(Opt, "time_delay", "1100");
#if 1
	cJSON_AddStringToObject(Opt, "apn_json", "{\"aps\":{\"alert\":\"Alarm: moved\", \"sound\":\"alarm.mp3\"}}");
#else
	//云巴的坑，不支持以下的写法
	cJSON_AddItemToObject(Opt,"apn_json",  apn_json=cJSON_CreateObject());
	cJSON_AddItemToObject(apn_json,"aps",  aps=cJSON_CreateObject());
	cJSON_AddStringToObject(aps,"alert",  "FENCE alarm");
	cJSON_AddStringToObject(aps,"sound",  "alarm.mp3");
#endif

    char *json = cJSON_PrintUnformatted(Opt);
	LOG_DEBUG("push to yunba: topic=%s, payload=%s, len=%d, opt=%s", topicName, payload, payloadLen, json);
    free(json);

	int rc = MQTTClient_publish2(client, topicName, payloadLen, payload, Opt);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("yunba push error: rc = %d", rc);
		return;
	}
	cJSON_Delete(Opt);

	return;
}

void yunba_publish(char *imei, int yunba_cmd, int status)
{
	char topic[128];
    memset(topic, 0, sizeof(topic));
    snprintf(topic, 128, "simcom_%s", imei);

	cJSON *Opt = cJSON_CreateObject();

	switch(yunba_cmd)
	{
		case YUNBA_CMD_ALARM:
			cJSON_AddStringToObject(Opt, "time_to_live", "120");
			cJSON_AddStringToObject(Opt, "time_delay", "1100");
			cJSON_AddStringToObject(Opt, "apn_json", "{\"aps\":{\"alert\":\"Alarm: moved\", \"sound\":\"alarm.mp3\"}}");
			break;

		case YUNBA_CMD_AUTOLOCK_NOTIFY:
			cJSON_AddStringToObject(Opt, "time_to_live", "120");
			cJSON_AddStringToObject(Opt, "time_delay", "1100");

			if(status == 1)
			{
				cJSON_AddStringToObject(Opt, "apn_json", "{\"aps\":{\"alert\":\"autolock notify: on\", \"sound\":\"default\"}}");
			}
			else if(status == 0)
			{
				cJSON_AddStringToObject(Opt, "apn_json", "{\"aps\":{\"alert\":\"autolock notify: off\", \"sound\":\"default\"}}");
			}
			
			break;

		default:
			break;
	}
	
	LOG_DEBUG("push to yunba: topic=%s, yunba_cmd=%d", topic, yunba_cmd);

	int rc = MQTTClient_publish2(client, topic, 0, NULL, Opt);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		LOG_ERROR("yunba push error: rc = %d", rc);

		if(rc == MQTTCLIENT_DISCONNECTED) //-3
		{
			LOG_INFO("reconnect to yunba");
			yunba_connect();
		}
		return;
	}
	cJSON_Delete(Opt);

	return;
}

