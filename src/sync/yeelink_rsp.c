/*
 * yeelink_rsp.c
 *
 *  Created on: Apr 30, 2015
 *      Author: jk
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yeelink_rsp.h"
#include "cb_ctx_mc.h"
#include "object_mc.h"
#include "cJSON.h"
#include "log.h"

size_t yeelink_onCreateDevice(void *contents, size_t size, size_t nmemb, void *userp)
{
	CB_CTX* ctx = userp;
	OBJ_MC* obj = ctx->obj;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);
	if (!json)
	{
		LOG_ERROR("error parse respone:%s", rsp);
	}
	else
	{
		int device_id = cJSON_GetObjectItem(json, "device_id")->valueint;
		obj->device_id = device_id;
		yeelink_createSensor(device_id, userp);
	}

	cJSON_Delete(json);
	free(rsp);


	return size * nmemb;
}

size_t yeelink_onCreateSensor(void *contents, size_t size, size_t nmemb, void *userp)
{
	CB_CTX* ctx = userp;
	OBJ_MC* obj = ctx->obj;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;


	cJSON* json = cJSON_Parse(rsp);
	if (!json)
	{
		LOG_ERROR("error parse respone:%s", rsp);
	}
	else
	{
		int sensor_id = cJSON_GetObjectItem(json, "sensor_id")->valueint;
		obj->sensor_id = sensor_id;
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t yeelink_onsaveGPS(void *contents, size_t size, size_t nmemb, void *userp)
{
	CB_CTX* ctx = userp;
	OBJ_MC* obj = ctx->obj;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	LOG_ERROR("error parse respone:%s", rsp);

	free(rsp);

	return size * nmemb;
}
