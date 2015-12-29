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
#include "db.h"
#include "macro.h"

#define LEANCLOUD_URL_BASE "https://api.leancloud.cn/1.1"

static int leancloud_post(CURL *curl, const char* class, const void* data, int len)
{
	char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); /* size of the POST data */

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
    	LOG_ERROR("leancloud_post failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

static int leancloud_batch(CURL *curl, const void* data, int len)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/batch", LEANCLOUD_URL_BASE);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); /* size of the POST data */

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_batch failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

static int leancloud_get(CURL *curl, const char* class)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s?limit=1000", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_get failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int leancloud_saveGPS(const char* imei, double lat, double lng)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "IMEI", imei);
	cJSON_AddNumberToObject(root, "lat",  lat);
	cJSON_AddNumberToObject(root, "lon",  lng);
    //cJSON_AddNumberToObject(root, "speed",	speed);
    //cJSON_AddNumberToObject(root, "course",	course);
    //cJSON_AddNumberToObject(root, "time", timestamp);
	char* data = cJSON_PrintUnformatted(root);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveGPS);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);
	int ret = leancloud_post(curl, "GPS", data, strlen(data));

	cJSON_Delete(root);
	free(data);

    return ret;
}

int leancloud_saveDid(const char* imei)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "IMEI", imei);
	char* data = cJSON_PrintUnformatted(root);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveDID);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, imei);
	int ret = leancloud_post(curl, "DID", data, strlen(data));

	cJSON_Delete(root);
	free(data);

    return ret;
}

int leancloud_makeMultiDidCurl(char** ppImeiMulti, int ImeiNum, char* data)
{
    cJSON *root, *requests, *request, *body;
    int ret = 0;

    root = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "requests", requests = cJSON_CreateArray());
    
    for(int i = 0; i < ImeiNum; i++)
    {
        cJSON_AddItemToArray(requests, request = cJSON_CreateObject());
        cJSON_AddStringToObject(request, "method", "POST");
        cJSON_AddStringToObject(request, "path", "/1.1/classes/DID");
        cJSON_AddItemToObject(request, "body", body = cJSON_CreateObject());
        cJSON_AddStringToObject(body, "IMEI", *(ppImeiMulti++));
    }

    data = cJSON_PrintUnformatted(root);
    LOG_INFO("%s", data);

    cJSON_Delete(root);

    return ret;
}

int leancloud_ResaveMultiDid_cb(void)
{
    //char a[10][16] = {0};
    char** ppImeiMulti = malloc(sizeof(10 * IMEI_LENGTH));
    int ImeiNum = 2;
    char* data = NULL;

    /*st_imeiMulti* pstImeiMulti = (st_imeiMulti*)malloc(sizeof(st_imeiMulti));
    if (!pstImeiMulti)
    {
        LOG_FATAL("memory alloc failed");
        return -1;
    }*/

    LOG_INFO("one-day timer for leancloud_ResaveMultiDid_cb");

    //get unposted IMEI
    /*
    ppImeiMulti[0] = "1234567890123457";
    ppImeiMulti[1] = "1234567890123458";
    ppImeiMulti[2] = "1234567890123459";*/

    //db_ResaveOBJUnpostedImei();

    LOG_INFO("hehe:%d", ImeiNum);
    
    //make multi DID curl
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveMultiDID);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    LOG_INFO("hehe");

    leancloud_makeMultiDidCurl(ppImeiMulti, ImeiNum, data);

    LOG_INFO("hehe");

    //batch to save multi DID
    int ret = leancloud_batch(curl, data, strlen(data));

    LOG_INFO("hehe");

    return ret;
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
