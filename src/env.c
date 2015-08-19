/*
 * env.c
 *
 *  Created on: May 27, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mosquitto.h>

#include "env.h"
#include "log.h"
#include "msg_proc_app.h"
#include "mqtt.h"

ENVIRONMENT env = {
    .mosq = NULL,
    .curl_leancloud = NULL,
    .curl_yeelink = NULL,
    .chunk = {NULL, 0},
};

void env_resetChunk(MemroyBuf* chunk)
{
	chunk->memory = realloc(chunk->memory, 1);
	chunk->size = 0;
}

void env_initial()
{
    char mqtt_id[32] = {0};
    pid_t self_pid = getpid();

    snprintf(mqtt_id, 32, "eelink_%d", self_pid);

    struct mosquitto* mosq = mqtt_login(mqtt_id, "server.xiaoan110.com", 1883,
                    app_log_callback,
                    app_connect_callback,
                    app_disconnect_callback,
                    app_message_callback,
                    app_subscribe_callback,
                    app_publish_callback,
                    &env);
    if (mosq)
    {
        LOG_INFO("connect to MQTT successfully");
        env.mosq = mosq;
    }
    else
    {
        LOG_ERROR("failed to connect to MQTT");
    }

    env.curl_leancloud = initCurlHandleOfLeancloud();
    env.curl_yeelink = initCurlHandleOfYeelink();

    env_resetChunk(&(env.chunk));
}

void env_cleanup()
{
    //cleanup all resource
    clcanupLeancloudHeader();
    cleanupYeelinkHeader();

    cleanupLeancloudCurlHandle(env.curl_leancloud);
    cleanupYeelinkCurlHandle(env.curl_yeelink);

    free(env.chunk.memory);
    
    
    if (env.mosq)
    {
        int rc = mosquitto_disconnect(env.mosq);
        if (rc != MOSQ_ERR_SUCCESS)
        {
                LOG_ERROR("mosq disconnect error:rc=%d", rc);
        }
        mosquitto_destroy(env.mosq);
    }
}

ENVIRONMENT* env_get()
{
	return &env;
}

