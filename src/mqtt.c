/*
 * mqtt.c
 *
 *  Created on: 2015��4��21��
 *      Author: jk
 */



#include <stdio.h>
#include <mosquitto.h>
#include "object_mc.h"
#include "log.h"

#define LOG_DEBUG(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_MQTT], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)



void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		printf("%s %s\n", message->topic, message->payload);
	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);

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

	 }

}

void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	OBJ_MC* obj = userdata;

	if(!result)
	{
		char topic[100];	//TODO: fix magic number
		memset(topic, 0, sizeof(topic));
		/* Subscribe to broker information topics on successful connect. */
		sprintf(topic, "ser2cli_noti/%s", PRODUCT_KEY);
		mosquitto_subscribe(mosq, NULL, topic, 2);

		sprintf(topic, "ser2cli_res/%s", obj->DID);
		mosquitto_subscribe(mosq, NULL, topic, 2);

		sprintf(topic, "app2dev/%s/#", obj->DID);
		mosquitto_subscribe(mosq, NULL, topic, 2);
	}
	else
	{
		fprintf(stderr, "Connect failed: result = %d", result);
	}
}

void mqtt_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(int i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void mqtt_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	printf("%s\n", str);
}

void mqtt_publish_callback(struct mosquitto *mosq, void *userdata, int mid)
{

}

struct mosquitto* mqtt_login(const char* id, const char* host, int port, void* ctx)
{
	int keepalive = 120;
	bool clean_session = true;

	LOG_DEBUG("login MQTT: id = %s,host=%s, port=%d", id, host, port);

	struct mosquitto *mosq = mosquitto_new(id, clean_session, ctx);
	if(!mosq)
	{
		LOG_ERROR("Error: Out of memory: mosquitto_new failed");
		return NULL;
	}
	mosquitto_log_callback_set(mosq, mqtt_log_callback);
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_message_callback_set(mosq, mqtt_message_callback);
	mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);
	mosquitto_publish_callback_set(mosq, mqtt_publish_callback);

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

