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
#include "objectID_leancloud.h"

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
    //curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
    	LOG_ERROR("leancloud_post failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

static int leancloud_sms(CURL *curl, const void *data, int len)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/requestSmsCode", "https://leancloud.cn:443/1.1");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); /* size of the POST data */

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_sms: %s", curl_easy_strerror(res));
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
    //curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_batch failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

static int leancloud_get(CURL *curl, const char *class)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s?limit=1000", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_get failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int leancloud_saveGPS(int timestamp, const char *imei, double lat, double lng, int speed, int course)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_saveGPS imei(%s), timestamp(%d), lat(%lf), lng(%lf), speed(%d), course(%d)",
             imei, timestamp, lat, lng, speed, course);

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "IMEI", imei);
	cJSON_AddNumberToObject(root, "lat",  lat);
	cJSON_AddNumberToObject(root, "lon",  lng);
    cJSON_AddNumberToObject(root, "speed",	speed);
    cJSON_AddNumberToObject(root, "course",	course);
    cJSON_AddNumberToObject(root, "time", timestamp);
	char* data = cJSON_PrintUnformatted(root);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveGPS);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);
	int ret = leancloud_post(curl, "GPS", data, strlen(data));

	cJSON_Delete(root);
	free(data);

    return ret;
}

int leancloud_saveGPSWithDID(int timestamp, const char *imei, double lat, double lng, int speed, int course, const char *did)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_saveGPSWithDID imei(%s), timestamp(%d), lat(%lf), lng(%lf), speed(%d), course(%d), did(%s)",
             imei, timestamp, lat, lng, speed, course, did);

    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "IMEI", imei);
    cJSON_AddNumberToObject(root, "lat",  lat);
    cJSON_AddNumberToObject(root, "lon",  lng);
    cJSON_AddNumberToObject(root, "speed", speed);
    cJSON_AddNumberToObject(root, "course",    course);
    cJSON_AddNumberToObject(root, "time", timestamp);
    cJSON_AddStringToObject(root, "did", did);
    char* data = cJSON_PrintUnformatted(root);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveGPS);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);
    int ret = leancloud_post(curl, "GPS", data, strlen(data));

    cJSON_Delete(root);
    free(data);

    return ret;
}

int leancloud_saveItinerary(const char *imei, int miles)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_saveItinerary imei(%s), miles(%d)",
             imei, miles);

    //get objectID from imei
    char *objectID = objectID_get_hash(imei);
    if(objectID == NULL)
    {
        LOG_ERROR("can't get objectId hash, imei(%s)", imei);

        //get objectID from leancloud
        leancloud_getObjectIDWithImei(imei);
        return -1;
    }

    //creat cjson put
    cJSON *put = cJSON_CreateObject();
    cJSON_AddStringToObject(put, "method", "PUT");

    char path[256] = {0};
    snprintf(path, 256, "/1.1/classes/DID/%s", objectID);
    cJSON_AddStringToObject(put, "path", path);

    cJSON *itinerary = cJSON_CreateObject();
    cJSON_AddStringToObject(itinerary, "__op", "Increment");
    cJSON_AddNumberToObject(itinerary, "amount", miles);

    cJSON *body = cJSON_CreateObject();
    cJSON_AddItemToObject(body, "itinerary", itinerary);
    cJSON_AddItemToObject(put, "body", body);

    //add put and post in cjson
    cJSON *requests = cJSON_CreateArray();
    cJSON_AddItemToArray(requests, put);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "requests", requests);

    char *data = cJSON_PrintUnformatted(root);
    LOG_DEBUG("%s", data);

    //set curl
    int ret = 0;
    char url[256] = {0};
    snprintf(url, 256, "%s/batch", LEANCLOUD_URL_BASE);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveItinerary);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_SaveItinerary failed: %s", curl_easy_strerror(res));
        ret = -1;
    }
    else
    {
        LOG_INFO("leancloud_SaveItinerary ok");
        ret = 0;
    }

    cJSON_Delete(root);
    free(data);

    return ret;
}

int leancloud_saveSimInfo(const char* imei, const char* ccid, const char* imsi)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_saveSimInfo imei(%s), ccid(%s), imsi(%s)",
             imei, ccid, imsi);

    //get objectID from imei
    char *objectID = objectID_get_hash(imei);
    if(objectID == NULL)
    {
        LOG_ERROR("can't get objectId hash, imei(%s)", imei);

        //get objectID from leancloud
        leancloud_getObjectIDWithImei(imei);
        return -1;
    }

    //creat cjson
    cJSON *put = cJSON_CreateObject();
    cJSON_AddStringToObject(put, "method", "PUT");

    char path[256] = {0};
    snprintf(path, 256, "/1.1/classes/DID/%s", objectID);
    cJSON_AddStringToObject(put, "path", path);

    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "CCID", ccid);
    cJSON_AddStringToObject(body, "IMSI", imsi);
    cJSON_AddItemToObject(put, "body", body);

    cJSON *requests = cJSON_CreateArray();
    cJSON_AddItemToArray(requests, put);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "requests", requests);
    
    char* data = cJSON_PrintUnformatted(root);
    LOG_DEBUG("%s", data);

    //set curl
    int ret = 0;
    char url[256] = {0};
    snprintf(url, 256, "%s/batch", LEANCLOUD_URL_BASE);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveSimInfo);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_SimInfo failed: %s", curl_easy_strerror(res));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    cJSON_Delete(root);
    free(data);

    return ret;
}

int leancloud_saveDid(const char* imei)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_saveDid imei(%s)", imei);

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

int leancloud_getObjectIDWithImei(const char* imei)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    LOG_INFO("leancloud_getObjectIDWithImei imei(%s)", imei);

    char url[256] = {0};
    snprintf(url, 256, "%s/classes/DID?where={\"IMEI\":\"%s\"}", LEANCLOUD_URL_BASE, imei);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onGetObjectIDWithImei);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    CURLcode res = curl_easy_perform(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_getObjectIDWithImei failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
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
    LOG_DEBUG("%s", data);

    cJSON_Delete(root);

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

int leancloud_getOBJ(void)
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
