/*
 * mqtt.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */


#include <stdio.h>
#include <mosquitto.h>
#include <string.h>

#include "log.h"
#include "macro.h"
#include "mqtt.h"

static struct mosquitto* mosq = NULL;


void mqtt_message_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata, const struct mosquitto_message *message)
{
    MQTT_ARG* mqtt_arg = (MQTT_ARG*)userdata;

    if(message->payloadlen)
    {
        LOG_DEBUG("%s %p", message->topic, message->payload);
    }
    else
    {
        LOG_ERROR("%s no payload(null)", message->topic);
    }

    LOG_INFO("recieve PUBLISH: %s", message->topic);

    if(strncmp(message->topic,"app2dev/",strlen("app2dev/")) == 0)
    {
        mqtt_arg->app_msg_handler(message->topic, message->payload, message->payloadlen, userdata);
    }
    else
    {
        LOG_ERROR("Receive unknown PUBLISH: %s", message->topic);
    }

}

void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int rc)
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

void mqtt_disconnect_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata, int rc)
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


void mqtt_subscribe_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int mid, int qos_count, const int *granted_qos)
{
    LOG_DEBUG("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(int i=1; i<qos_count; i++){
        LOG_DEBUG(", %d", granted_qos[i]);
    }
}

void mqtt_log_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int level, const char *str)
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

void mqtt_publish_callback(struct mosquitto *mosq __attribute__((unused)), void *userdata __attribute__((unused)), int mid __attribute__((unused)))
{
    LOG_INFO("Publish mid: %d successfully", mid);
}

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
	mosquitto_reconnect_delay_set(mosq, 10, 120, false);

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

void mqtt_initial(MQTT_ARG* mqtt_arg)
{
	mosq = mqtt_login("electrombile", "127.0.0.1", 1883,
										mqtt_log_callback,
										mqtt_connect_callback,
										mqtt_disconnect_callback,
										mqtt_message_callback,
										mqtt_subscribe_callback,
										mqtt_publish_callback,
										mqtt_arg);
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
	LOG_INFO("mosq pub succeed: IMEI(%s)", topic);
}

void mqtt_subscribe(const char *imei)
{
	char topic[IMEI_LENGTH + 20];
	memset(topic, 0, sizeof(topic));

	snprintf(topic, IMEI_LENGTH + 20, "app2dev/%s/cmd", (char *)imei);
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

	snprintf(topic, IMEI_LENGTH + 20, "app2dev/%s/+/cmd", (char *)imei);
    LOG_INFO("unsubscribe topic: %s", topic);
	mosquitto_unsubscribe(mosq, NULL, topic);
}

