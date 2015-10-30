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
#include "mqtt.h"

ENVIRONMENT env = {
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
}

ENVIRONMENT* env_get()
{
	return &env;
}

