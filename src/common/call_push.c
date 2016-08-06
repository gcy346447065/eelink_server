/*
 * call_push.c
 *
 *  Created on: July 22, 2016
 *      Author: lc
 */


#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "call_push.h"
#include "log.h"
#include "cencode.h"
#include "md5.h"

static struct curl_slist *getYunzxHeader(char *timeString)
{
    char base64[64];
    char Authorization[80];

    snprintf(base64, 64, "2155bf74725e01c68e3ae717fa14e13b:%s", timeString);
    snprintf(Authorization, 80, "Authorization:%s", encode(base64));

    static struct curl_slist *headerlist = NULL;
    if(!headerlist)
    {
        headerlist = curl_slist_append(headerlist, "Accept:application/json");
        headerlist = curl_slist_append(headerlist, "Content-Type:application/xml;charset=utf-8");
        headerlist = curl_slist_append(headerlist, "Content-Length:256");
        headerlist = curl_slist_append(headerlist, Authorization);
    }

    return headerlist;
}

static char *md5Encode(char *timeString)
{
    MD5_CTX md5;
    unsigned char result[32];
    unsigned char encrypt[80];
    snprintf(encrypt, 80, "2155bf74725e01c68e3ae717fa14e13b+token+%s", timeString);
    MD5Init(&md5);
    MD5Update(&md5, encrypt, strlen((char *)encrypt));
    MD5Final(&md5, result);
    return result;
};

static char *getYunzxURL(char *timeString)
{
    char md5[64];
    snprintf(md5,64,"https://api.ucpaas.com/2014-06-30/Accounts/2155bf74725e01c68e3ae717fa14e13b/Calls/voiceNotify?sig=%s",md5Encode(timeString));
}


size_t call_Send_onPush(void *contents, size_t size, size_t nmemb, void *userdata)
{
    userdata = userdata;

    char* rsp = malloc(size * nmemb + 1);
    memcpy(rsp, contents, size * nmemb);
    rsp[size * nmemb] = 0;

    cJSON* json = cJSON_Parse(rsp);

    if (!json)
    {
        LOG_ERROR("error call message response: %s", rsp);
    }
    else
    {
        LOG_DEBUG("get call message response: %s", rsp);
    }

    cJSON_Delete(json);
    free(rsp);

    return size * nmemb;
}

int call_Send(char *number)//TODO: need a project id
{
    int ret = 0;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        cJSON *root = cJSON_CreateObject();
        cJSON *voiceNotify = cJSON_CreateObject();

        cJSON_AddStringToObject(voiceNotify, "appId", "8a216da85610bfb80156179b4ae60564");
        cJSON_AddStringToObject(voiceNotify, "to", number);
        cJSON_AddStringToObject(voiceNotify, "type", "0");
        cJSON_AddStringToObject(voiceNotify, "content", "≤‚ ‘");
        cJSON_AddStringToObject(voiceNotify, "toSerNum", "075512345678");
        cJSON_AddStringToObject(voiceNotify, "playTimes", "3");

        cJSON_AddItemToObject(root, "voiceNotify", voiceNotify);

        char *data = cJSON_PrintUnformatted(root);

        LOG_DEBUG("%s", data);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, getYunzxURL("20160806115030"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, getYunzxHeader("20160806102530"));

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_Send_onPush);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            LOG_ERROR("call alarm send failed: %s", curl_easy_strerror(res));
            ret = -2;
        }

        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_ERROR("call alarm send curl_easy_init failed");
        ret = -1;
    }

    LOG_DEBUG("call alarm send: number(%s)", number);

    return ret;
}





