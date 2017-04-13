/*
 * mqtt.c
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */


#include <stdio.h>
#include <mosquitto.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/util.h>

#include "log.h"
#include "macro.h"
#include "mqtt.h"
#include "timer.h"
#include "setting.h"

static struct mosquitto *mosq = NULL;
static struct event *evTimerReconnect = NULL;

void mqtt_message_callback(struct mosquitto *m __attribute__((unused)), void *userdata, const struct mosquitto_message *message)
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


    if(strncmp(message->topic, "app2dev/", strlen("app2dev/")) == 0)
    {
        LOG_INFO("recieve app2dev PUBLISH: %s", message->topic);
        mqtt_arg->app_msg_handler(message->topic, message->payload, message->payloadlen);
    }
    else if(strncmp(message->topic, "dev2app/", strlen("dev2app/")) == 0)
    {
        LOG_INFO("Receive dev2app PUBLISH: %s", message->topic);
        mqtt_arg->app_msg_handler(message->topic, message->payload, message->payloadlen);
    }
    else
    {
        LOG_ERROR("Receive unknown PUBLISH: %s", message->topic);
    }

    return;
}

void mqtt_connect_callback(struct mosquitto *m __attribute__((unused)), void *userdata __attribute__((unused)), int rc)
{
    if(!rc)
    {
        LOG_INFO("Connect to MQTT server successfully");
    }
    else
    {
        LOG_ERROR("Connect failed: result = %s", mosquitto_connack_string(rc));
    }

    return;
}

void mqtt_disconnect_callback(struct mosquitto *m __attribute__((unused)), void *userdata __attribute__((unused)), int rc)
{
    if(!rc)
    {
        LOG_INFO("client disconnect successfully");
    }
    else
    {
        LOG_ERROR("disconnect rc = %d(%s)\n",  rc, mosquitto_strerror(rc));

        //it will auto reconnect
        //mosquitto_reconnect(mosq);
    }

    return;
}

void mqtt_subscribe_callback(struct mosquitto *m , void *userdata , int mid, int qos_count, const int *granted_qos)
{
    m = m;
    userdata = userdata;

    LOG_DEBUG("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(int i=1; i<qos_count; i++)
    {
        LOG_DEBUG(", %d", granted_qos[i]);
    }

    return;
}

void mqtt_log_callback(struct mosquitto *m , void *userdata , int level, const char *str)
{
    m = m;
    userdata = userdata;

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
            break;
    }

    return;
}

void mqtt_publish_callback(struct mosquitto *m __attribute__((unused)), void *userdata __attribute__((unused)), int mid)
{
    m = m;
    userdata = userdata;

    LOG_INFO("Publish mid: %d successfully", mid);

    return;
}

void mqtt_reconnect_cb(evutil_socket_t fd __attribute__((unused)), short a __attribute__((unused)), void * arg)
{
    fd = fd;
    a = a;

    LOG_INFO("re-connect to MQTT server");
    mqtt_initial(arg);

    return;
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
    MQTT_ARG *arg = ctx;

	LOG_INFO("login MQTT: id = %s,host=%s, port=%d", id, host, port);

	struct mosquitto *m = mosquitto_new(id, clean_session, arg);
	if(!m)
	{
		LOG_ERROR("Error: Out of memory, mosquitto_new failed");
		return NULL;
	}
	mosquitto_log_callback_set(m, on_log);
	mosquitto_connect_callback_set(m, on_connect);
	mosquitto_disconnect_callback_set(m, on_disconnect);
	mosquitto_message_callback_set(m, on_message);
	mosquitto_subscribe_callback_set(m, on_subscribe);
	mosquitto_publish_callback_set(m, on_publish);
	mosquitto_reconnect_delay_set(m, 10, 120, false);

//	LOG_DEBUG("set MQTT username:%s, password:%s", get_IMEI_STRING(obj->DID), obj->pwd);
//	mosquitto_username_pw_set(m, get_IMEI_STRING(obj->DID), obj->pwd);

	int rc = mosquitto_connect(m, host, port, keepalive);
	if(rc != MOSQ_ERR_SUCCESS)
	{
		LOG_ERROR("%s connect to %s:%d failed:%s", id, host, port, mosquitto_strerror(rc));

        mosquitto_destroy(m);

        struct timeval one_minitue = { 60, 0 };

        if (!evTimerReconnect)
        {
            evTimerReconnect = timer_newLoop(arg->base, &one_minitue, mqtt_reconnect_cb, arg);
        }
		return NULL;
	}
	else
	{
		LOG_INFO("MC:%s connect to %s:%d successfully", id, host, port);
	}

    mosquitto_loop_start(m);

	return m;
}

void mqtt_initial(MQTT_ARG* mqtt_arg)
{
    char *mqtt_id = 0;
    char *host = setting.mqtt_host;
    int port = setting.mqtt_port;

    char hostname[256] = {0};
    gethostname(hostname, 256);
    hostname[255] = 0;

    size_t len = strlen(hostname) + 10;
    mqtt_id = malloc(len);
    if (!mqtt_id)
    {
        LOG_FATAL("can not alloc memory");
        exit(0);
    }

    snprintf(mqtt_id, len, "%s-%d", hostname, getpid());

    mosq = mqtt_login(mqtt_id, host, port,
                      mqtt_log_callback,
                      mqtt_connect_callback,
                      mqtt_disconnect_callback,
                      mqtt_message_callback,
                      mqtt_subscribe_callback,
                      mqtt_publish_callback,
                      mqtt_arg);

    free(mqtt_id);
    if (mosq)
    {
        LOG_INFO("connect to MQTT successfully");
        if (evTimerReconnect)
        {
            evtimer_del(evTimerReconnect);
            event_free(evTimerReconnect);
            evTimerReconnect = NULL;
        }
    }
    else
    {
        LOG_ERROR("failed to connect to MQTT");
    }

    return;
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

    if (evTimerReconnect)
    {
        evtimer_del(evTimerReconnect);
        event_free(evTimerReconnect);
    }

    return;
}

void mqtt_publish(const char *topic, const void *payload, size_t payloadlen)
{
	int rc = mosquitto_publish(mosq, NULL, topic, payloadlen, payload, 0, false);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		LOG_ERROR("mosq pub error: rc = %d(%s)", rc, mosquitto_strerror(rc));
        return;
	}
	LOG_INFO("mosq pub succeed: IMEI(%s)", topic);

    return;
}

void mqtt_subscribe(const char *imei)
{
	char topic[IMEI_LENGTH + 13];
	memset(topic, 0, sizeof(topic));

	snprintf(topic, IMEI_LENGTH + 13, "app2dev/%s/cmd", (char *)imei);
	int rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if(MOSQ_ERR_SUCCESS == rc)
	{
		LOG_INFO("subscribe topic: %s", topic);
	}
	else
	{
		LOG_ERROR("subscribe topic: %s error %d", topic, rc);
	}

    return;
}

void mqtt_unsubscribe(const char *imei)
{
	char topic[IMEI_LENGTH + 15];
	memset(topic, 0, sizeof(topic));

	snprintf(topic, IMEI_LENGTH + 15, "app2dev/%s/+/cmd", (char *)imei);
    LOG_INFO("unsubscribe topic: %s", topic);
	mosquitto_unsubscribe(mosq, NULL, topic);

    return;
}

int mqtt_subscribe_allGPS(void)
{
	char topic[] = "dev2app/+/gps";

	int rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if(MOSQ_ERR_SUCCESS != rc)
	{
		LOG_ERROR("subscribe topic: %s error %d", topic, rc);
        return -1;
	}

    LOG_INFO("subscribe topic: %s", topic);
    return 0;
}

int mqtt_unsubscribe_allGPS(void)
{
	char topic[] = "dev2app/+/gps";

    LOG_INFO("unsubscribe topic: %s", topic);
	mosquitto_unsubscribe(mosq, NULL, topic);

    return 0;
}


