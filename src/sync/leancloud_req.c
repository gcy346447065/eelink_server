/*
 * leancloud_req.c
 *
 *  Created on: Apr 25, 2015
 *      Author: jk
 */

#include <string.h>
#include <stdlib.h>

#include "leancloud_req.h"
#include "leancloud_rsp.h"
#include "cJSON.h"
#include "curl.h"
#include "log.h"
#include "env.h"

#define LEANCLOUD_URL_BASE "https://api.leancloud.cn/1.1"

static struct event *evTimerReconnect = NULL;

static void leancloud_post(CURL *curl, const char* class, const void* data, int len)
{
	char url[256] = {0};

	snprintf(url, 256, "%s/classes/%s", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); /* size of the POST data */

    //LOG_INFO("Post leancloud: %s ---> %s", url, data);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
    	LOG_ERROR("curl_easy_perform() failed: %s",curl_easy_strerror(res));
    }

    return;
}

static int leancloud_get(CURL *curl, const char* class)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s?limit=1000", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("curl_easy_perform() failed: %s",curl_easy_strerror(res));
        return -1;
    }
    return 0;
}

void leancloud_saveGPS(const char* imei, double lat, double lng)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root,"IMEI", imei);
	cJSON_AddNumberToObject(root,"lat",	lat);
	cJSON_AddNumberToObject(root,"lon",	lng);
//	cJSON_AddNumberToObject(root,"speed",	speed);
//	cJSON_AddNumberToObject(root,"course",	course);
//	cJSON_AddNumberToObject(root,"time", timestamp);
	char* data = cJSON_PrintUnformatted(root);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveGPS);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);
	leancloud_post(curl, "GPS", data, strlen(data));
	cJSON_Delete(root);
	free(data);
}

void leancloud_saveDid(const char* imei)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root,"IMEI", imei);
	char* data = cJSON_PrintUnformatted(root);
	leancloud_post(curl, "DID", data, strlen(data));
	cJSON_Delete(root);
	free(data);
}

int leancloud_onGetOBJ(MemroyBuf *chunk)
{
    int ret = 0;;

    cJSON* root = cJSON_Parse(chunk->memory);
    if (!root)
    {
        LOG_ERROR("error parse respone");
        return -1;
    }

	/* get the array from array name */
    cJSON* pResults = cJSON_GetObjectItem(root, "results");
	if(!pResults)
	{
		LOG_ERROR("error get json array");
		cJSON_Delete(root);
        return -1;
	}

	int iSize = cJSON_GetArraySize(pResults);
	for(int i = 0; i < iSize; i++)
	{
		cJSON* pSub = cJSON_GetArrayItem(pResults, i);
		if(NULL == pSub)
		{
			LOG_ERROR("error GetArrayItem");
			continue;
		}

		cJSON* imei = cJSON_GetObjectItem(pSub, "IMEI");
		cJSON* did = cJSON_GetObjectItem(pSub, "did");
		cJSON* password = cJSON_GetObjectItem(pSub, "password");

		if (!imei || !did || !password)
		{
			LOG_ERROR("parse json error");
			continue;
		}

		LOG_DEBUG("initil IMEI(%s)", imei->valuestring);

	}


    cJSON_Delete(root);
    return ret;
}

int leancloud_getOBJ()
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;
    MemroyBuf* chunk = &(env->chunk);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onRev);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, chunk);

    int ret = leancloud_get(curl, "DID");
    if (ret)
    {
        LOG_ERROR("get DID failed");
        return -1;
    }

    ret = leancloud_onGetOBJ(chunk);

    env_resetChunk(chunk);

    if (ret)
    {
        return -1;
    }
    return 0;
}
