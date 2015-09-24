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
#include "cb_ctx_mc.h"
#include "object_mc.h"
#include "cJSON.h"
#include "log.h"

#define LEANCLOUD_URL_BASE "https://api.leancloud.cn/1.1"

static void leancloud_post(CURL *curl, const char* class, const void* data, int len)
{
	char url[256] = {0};


	snprintf(url, 256, "%s/classes/%s", LEANCLOUD_URL_BASE, class);

    curl_easy_setopt(curl, CURLOPT_URL, url);

	curl_easy_setopt(curl, CURLOPT_POST, 1L);

	/* pass in a pointer to the data - libcurl will not copy */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	/* size of the POST data */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);

    LOG_INFO("Post leancloud: url->%s, data->%s", url, data);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);


    /* Check for errors */
    if(res != CURLE_OK)
    {
    	LOG_ERROR("curl_easy_perform() failed: %s",curl_easy_strerror(res));
    }

    //cleanup when the connect is down, see server_mc.c
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


void leancloud_saveGPS(OBJ_MC* obj)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root,"IMEI",    get_IMEI_STRING(obj->IMEI));
	cJSON_AddStringToObject(root,"did", 	obj->DID);
	cJSON_AddNumberToObject(root,"lat",	obj->lat / 30000.0);
	cJSON_AddNumberToObject(root,"lon",	obj->lon / 30000.0);
	cJSON_AddNumberToObject(root,"speed",	obj->speed);
	cJSON_AddNumberToObject(root,"course",	obj->course);
	cJSON_AddNumberToObject(root,"time",obj->timestamp);
	char* data = cJSON_PrintUnformatted(root);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, leancloud_onSaveGPS);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, env);
	leancloud_post(curl, "GPS", data, strlen(data));
	cJSON_Delete(root);
	free(data);
//	char data[100] = {0};	//use macro
//	snprintf(data, 100, "{\"did\":\"%s\",\"lat\":%d,\"lon\":%d,\"speed\":%d,\"course\":%d}", obj->DID, obj->lat, obj->lon, obj->speed, obj->course);
//	leancloud_post(curl, data, strlen(data));
}

void leancloud_saveDid(OBJ_MC* obj)
{
	ENVIRONMENT* env = env_get();
	CURL* curl = env->curl_leancloud;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root,"did", 	obj->DID);
	cJSON_AddStringToObject(root,"IMEI",	get_IMEI_STRING(obj->IMEI));
	cJSON_AddStringToObject(root,"password",obj->pwd);
	char* data = cJSON_PrintUnformatted(root);
	leancloud_post(curl, "DID", data, strlen(data));
	cJSON_Delete(root);
	free(data);
}

int leancloud_onGetOBJ(MemroyBuf *chunk)
{
    int ret = 0;;
    OBJ_MC* obj;

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

		obj = mc_obj_new();
		if (NULL == obj)
		{
			LOG_FATAL("insufficient memory when new a obj");
		    cJSON_Delete(root);
		    return -1;
		}
		memcpy(obj->IMEI, get_IMEI(imei->valuestring), IMEI_LENGTH);
		memcpy(obj->DID, did->valuestring, MAX_DID_LEN);
		memcpy(obj->pwd, password->valuestring, MAX_PWD_LEN);

		/* add to mc hash */
		mc_obj_add(obj);

		//subscribe
		app_subscribe(env_get()->mosq, obj);
	}


    cJSON_Delete(root);
    return ret;
}


/* get obj config */
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
