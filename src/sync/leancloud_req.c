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
    //curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
    	LOG_ERROR("leancloud_post failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

size_t update_callback(void* buff, size_t size, size_t nmemb, void* userp) 
{ 
    size_t sizes = fread(buff, size, nmemb, (FILE *)userp); 
    return sizes; 
}

static int leancloud_update(CURL *curl, const char* class, const char* objectID, const void* data, int len)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s/%s", LEANCLOUD_URL_BASE, class, objectID);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);//new version or CURLOPT_PUT
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, &update_callback);
    //curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
    //curl_easy_setopt(curl, CURLOPT_INFILESIZE, fsize);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    //curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_update failed: %s", curl_easy_strerror(res));
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

static int leancloud_get(CURL *curl, const char* class)
{
    char url[256] = {0};

    snprintf(url, 256, "%s/classes/%s?limit=1000", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    CURLcode res = curl_easy_perform(curl);
    //curl_easy_cleanup(curl);
    if(CURLE_OK != res)
    {
        LOG_ERROR("leancloud_get failed: %s", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int leancloud_saveGPS(int timestamp, const char* imei, double lat, double lng, double speed, double course)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

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

int leancloud_saveSimInfo(const char* imei, const char* ccid, const char* imsi)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "CCID", ccid);
    cJSON_AddStringToObject(root, "IMSI", imsi);
    char* data = cJSON_PrintUnformatted(root);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveSimInfo);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, imei);

    //get objectID with imei from hash table
    const char* objectID;
    int ret = leancloud_update(curl, "DID", objectID, data, strlen(data));

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

int leancloud_sendSms2Tel(const char *SmsTemplate, const char *TelNumber)
{
    ENVIRONMENT* env = env_get();
    CURL* curl = env->curl_leancloud;

    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "mobilePhoneNumber", "15871413731");
    cJSON_AddStringToObject(root, "template", "SmsAlarm");
    char *data = cJSON_PrintUnformatted(root);

    int ret = leancloud_sms(curl, data, strlen(data));

    cJSON_Delete(root);
    free(data);

    return ret;
}
