/*
 * mqtt.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */


#include <stdio.h>
#include <string.h>
#include <mosquitto.h>

#include "object_mc.h"
#include "log.h"

#ifdef WITH_CATEGORY

#define LOG_DEBUG(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNING(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)

#endif


struct mosquitto* mqtt_login(const char* id, const char* host, int port,
		void (*on_log)(struct mosquitto *, void *, int, const char *),
		void (*on_connect)(struct mosquitto *, void *, int),
		void (*on_disconnect)(struct mosquitto *, void *, int),
		void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *),
		void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *),
		void (*on_publish)(struct mosquitto *, void *, int),
		void* ctx)
{
	int keepalive = 200;
	bool clean_session = true;

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
	mosquitto_reconnect_delay_set(mosq, 10, 120, false);

//	OBJ_MC* obj = ctx;

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

