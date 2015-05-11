/*
 * mqtt.c
 *
 *  Created on: 2015��4��21��
 *      Author: jk
 */



#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include "object_mc.h"
#include "log.h"
#include "gizwits_rsp.h"

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

void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		LOG_DEBUG("%s %p", message->topic, message->payload);
	}else{
		LOG_DEBUG("%s no payload(null)", message->topic);
	}
	fflush(stdout);

	 LOG_DEBUG("recieve PUBLISH: %s", message->topic);

	 if(strncmp(message->topic,"app2dev/",strlen("app2dev/"))==0)
	 {
		 mqtt_app2dev(message->topic, message->payload, message->payloadlen, userdata);
	 }
	 else if(strncmp(message->topic,"ser2cli_res/",strlen("ser2cli_res/"))==0)
	 {
		 mqtt_ser2cli_res(message->topic, message->payload, message->payloadlen, userdata);
	 }
	 else if(strncmp(message->topic,"ser2cli_noti/",strlen("ser2cli_noti/"))==0)
	 {
		 mqtt_ser2cli_noti(message->topic, message->payload, message->payloadlen, userdata);
	 }
	 else
	 {
		 LOG_ERROR("recieve unknown PUBLISH");
	 }

}

void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	OBJ_MC* obj = userdata;

	if(!result)
	{
		char topic[100];	//TODO: fix magic number
		memset(topic, 0, sizeof(topic));
		snprintf(topic, 100, "ser2cli_noti/%s", PRODUCT_KEY);
		/* Subscribe to broker information topics on successful connect. */
		LOG_INFO(topic, "ser2cli_noti/%s", PRODUCT_KEY);
		mosquitto_subscribe(mosq, NULL, topic, 0);

		snprintf(topic, 100, "ser2cli_res/%s", obj->DID);
		LOG_INFO(topic, "ser2cli_res/%s", obj->DID);
		mosquitto_subscribe(mosq, NULL, topic, 0);

		snprintf(topic, 100, "app2dev/%s/#", obj->DID);
		LOG_INFO(topic, "app2dev/%s/#", obj->DID);
		mosquitto_subscribe(mosq, NULL, topic, 0);
	}
	else
	{
		LOG_ERROR("Connect failed: result = %d", result);
	}
}
void mqtt_disconnect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	OBJ_MC* obj = userdata;

	if(rc)
	{
		LOG_ERROR("%s disconnect rc = %d(%s)\n", obj->DID, rc, mosquitto_strerror(rc));

		//mosquitto_reconnect(mosq);
	}
	else
	{
		LOG_ERROR("client disconnect\n");	}
}


void mqtt_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	LOG_DEBUG("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(int i=1; i<qos_count; i++){
		LOG_DEBUG(", %d", granted_qos[i]);
	}
}

void mqtt_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	switch (level)
	{
	case MOSQ_LOG_DEBUG:
		LOG_DEBUG("%s", str);

		break;
	case MOSQ_LOG_INFO:
		LOG_INFO("%s", str);
		break;

	case MOSQ_LOG_NOTICE:
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

void mqtt_publish_callback(struct mosquitto *mosq, void *userdata, int mid)
{
	OBJ_MC* obj = userdata;

}

struct mosquitto* mqtt_login(const char* id, const char* host, int port, void* ctx)
{
	int keepalive = 200;
	bool clean_session = false;

	LOG_DEBUG("login MQTT: id = %s,host=%s, port=%d", id, host, port);

	struct mosquitto *mosq = mosquitto_new(id, clean_session, ctx);
	if(!mosq)
	{
		LOG_ERROR("Error: Out of memory: mosquitto_new failed");
		return NULL;
	}
	mosquitto_log_callback_set(mosq, mqtt_log_callback);
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_disconnect_callback_set(mosq, mqtt_disconnect_callback);
	mosquitto_message_callback_set(mosq, mqtt_message_callback);
	mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);
	mosquitto_publish_callback_set(mosq, mqtt_publish_callback);
	mosquitto_reconnect_delay_set(mosq, 10, 120, false);

	OBJ_MC* obj = ctx;

	LOG_DEBUG("set MQTT username:%s, password:%s", obj->DID, obj->pwd);
	mosquitto_username_pw_set(mosq, obj->DID, obj->pwd);

	if(mosquitto_connect(mosq, host, port, keepalive))
	{
		LOG_ERROR("Unable to connect.");
		return NULL;
	}

    mosquitto_loop_start(mosq);

//	mosquitto_destroy(mosq);
	return mosq;
}

