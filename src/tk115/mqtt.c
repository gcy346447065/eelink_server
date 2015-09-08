/*
 * mqtt.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */


#include <stdio.h>
#include <mosquitto.h>
#include <string.h>

#include "msg_proc_app.h"
#include "log.h"
#include "macro.h"

static struct mosquitto* mosq = NULL;

static struct mosquitto* mqtt_login(const char* id, const char* host, int port,
		void (*on_log)(struct mosquitto *, void *, int, const char *),
		void (*on_connect)(struct mosquitto *, void *, int),
		void (*on_disconnect)(struct mosquitto *, void *, int),
		void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *),
		void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *),
		void (*on_publish)(struct mosquitto *, void *, int),
		void* ctx)
{
	int keepalive = 200;
	bool clean_session = false;

	LOG_INFO("login MQTT: id = %s,host=%s, port=%d", id, host, port);

	struct mosquitto *mosq = mosquitto_new(id, clean_session, ctx);
	if(!mosq)
	{
		LOG_ERROR("Error: Out of memory, mosquitto_new failed");
		return NULL;
	}
	mosquitto_log_callback_set(mosq, on_log);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_message_callback_set(mosq, on_message);
	mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_publish_callback_set(mosq, on_publish);
//	mosquitto_reconnect_delay_set(mosq, 10, 120, false);

//	OBJECT* obj = ctx;

//	LOG_DEBUG("set MQTT username:%s, password:%s", get_IMEI_STRING(obj->DID), obj->pwd);
//	mosquitto_username_pw_set(mosq, get_IMEI_STRING(obj->DID), obj->pwd);

	int rc = mosquitto_connect(mosq, host, port, keepalive);
	if(rc != MOSQ_ERR_SUCCESS)
	{
		//TODO: to check whether mosquitto_connack_string here is OKs
		LOG_ERROR("MC:%s connect to %s:%d failed:%s", id, host, port, mosquitto_strerror(rc));
		return NULL;
	}
	else
	{
		LOG_INFO("MC:%s connect to %s:%d successfully", id, host, port);
	}

    mosquitto_loop_start(mosq);

//	mosquitto_destroy(mosq);
	return mosq;
}

void mqtt_initial()
{
	mosq = mqtt_login("elink", "127.0.0.1", 1883,
										app_log_callback,
										app_connect_callback,
										app_disconnect_callback,
										app_message_callback,
										app_subscribe_callback,
										app_publish_callback,
										NULL);
    //TODO: to fix the above user data
	if (mosq)
	{
		LOG_INFO("connect to MQTT successfully");
	}
	else
	{
		LOG_ERROR("failed to connect to MQTT");
	}

}

void mqtt_cleanup()
{
	if (mosq)
	{
		int rc = mosquitto_disconnect(mosq);
		if (rc != MOSQ_ERR_SUCCESS)
		{
			LOG_ERROR("mosq disconnect error:rc=%d", rc);
		}
		mosquitto_destroy(mosq);
	}
}

void mqtt_publish(const char *topic, const void *payload, int payloadlen)
{
	int rc = mosquitto_publish(mosq, NULL, topic, payloadlen, payload, 0, false);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		LOG_ERROR("mosq pub error: rc = %d(%s)", rc, mosquitto_strerror(rc));
	}
}

void mqtt_subscribe(const char *imei)
{
	char topic[IMEI_LENGTH + 20];
	memset(topic, 0, sizeof(topic));

	snprintf(topic, IMEI_LENGTH + 20, "app2dev/%s/e2link/cmd", (char *)imei);
	int rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if(MOSQ_ERR_SUCCESS == rc)
	{
		LOG_INFO("subscribe topic: %s", topic);
	}
	else
	{
		LOG_ERROR("subscribe topic: %s error", topic);
	}
}

void mqtt_unsubscribe(const char *imei)
{
	char topic[IMEI_LENGTH + 20];
	memset(topic, 0, sizeof(topic));

	snprintf(topic, IMEI_LENGTH + 20, "app2dev/%s/e2link/cmd", (char *)imei);
    LOG_INFO("unsubscribe topic: %s", topic);
	mosquitto_unsubscribe(mosq, NULL, topic);
}


