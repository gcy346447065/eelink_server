/*
 * leancloud_rsp.c
 *
 *  Created on: May 1, 2015
 *      Author: jk
 */
#include <stdlib.h>
#include <string.h>

#include "leancloud_rsp.h"
#include "leancloud_req.h"
#include "cJSON.h"
#include "log.h"
#include "env.h"
#include "db.h"

size_t leancloud_onSaveGPS(void *contents, size_t size, size_t nmemb, void *userdata)
{
	userdata = userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse response: %s", rsp);
	}
	else
	{
		LOG_INFO("get save GPS response: %s", rsp);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onSaveDID(void *contents, size_t size, size_t nmemb, void *userdata)
{
	const char* imei = (const char*) userdata;

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse response: %s", rsp);
	}
	else
	{
		LOG_INFO("get save DID response: %s", rsp);

		db_updateOBJIsPosted(imei);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onSaveMultiDID(void *contents, size_t size, size_t nmemb, void *userdata)
{
	//struct st_imeiMulti* pstImeiMulti = (struct st_imeiMulti*) userdata;

	st_imeiMulti* pstImeiMulti = malloc(sizeof(st_imeiMulti));
	memcpy(pstImeiMulti, userdata, sizeof(st_imeiMulti));

	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse response: %s", rsp);
	}
	else
	{
		LOG_INFO("get save DID response: %s", rsp);

		LOG_INFO("pstImeiMulti->imeiNum = %d", pstImeiMulti->imeiNum);
		//db_updateOBJIsPosted(imei);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onRev(void *contents, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size * nmemb;
	MemroyBuf *mem = (MemroyBuf *)userdata;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL)
	{
		LOG_ERROR("not enough memory (realloc returned NULL)");
		return 0;
	}

	LOG_DEBUG("receive %zu bytes from leancloud", realsize);

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}
