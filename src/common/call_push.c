/*
 * call_push.c
 *
 *  Created on: July 22, 2016
 *      Author: lc
 */


#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>

#include "call_push.h"
#include "log.h"
#include "cencode.h"
#include "md5.h"

static struct curl_slist *getYunzxHeader(char *timeString)
{
    char Authorization[80] = {0};
    char base64[65] = {0};
    static struct curl_slist *headerlist = NULL;

    snprintf(base64, 65, "2155bf74725e01c68e3ae717fa14e13b:%14s", timeString);//accountSid:time base64
    snprintf(Authorization, 80, "Authorization: %64s", encode(base64));
    Authorization[strlen(Authorization)] = 0;
    if(!headerlist)
    {
        headerlist = curl_slist_append(headerlist, "Accept: application/json");
        headerlist = curl_slist_append(headerlist, "Content-Type: application/json;charset=utf-8");
        headerlist = curl_slist_append(headerlist, Authorization);
    }

    return headerlist;
}

static void MD5encode(char *timeString, char *result)
{
    MD5_CTX md5 = {0};
    char encrypt[85] = {0};
    snprintf(encrypt, 85, "2155bf74725e01c68e3ae717fa14e13b902948affd07940b8bb09637d7745c55%14s", timeString);//account+Sidtoken+time md5
    MD5Init(&md5);
    MD5Update(&md5, encrypt, strlen(encrypt));
    MD5Final(&md5, result);
    result[32] = 0;
    LOG_DEBUG("%d:%s", strlen(encrypt), encrypt);
};

static void get_timeString(char *timeString)
{
    long ts;
    struct tm *ptm;

    ts = time(NULL);
    ptm = localtime(&ts);
    snprintf(timeString, 15, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

static void get_YunzxURL(char *timeString, char *URL)
{
    char result[33] = {0};
    MD5encode(timeString,result);
    snprintf(URL,256,"https://api.ucpaas.com/2014-06-30/Accounts/2155bf74725e01c68e3ae717fa14e13b/Calls/voiceNotify?sig=%32s",result);
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

int call_Send(char *number)
{
    int ret = 0;

    char URL[256] = {0};
    char timeString[15] = {0};

    get_timeString(timeString);
    get_YunzxURL(timeString, URL);
    timeString[14] = 0;
    URL[strlen(URL)] = 0;

    CURL *curl = curl_easy_init();
    if(curl)
    {
        cJSON *root = cJSON_CreateObject();
        cJSON *voiceNotify = cJSON_CreateObject();

        cJSON_AddStringToObject(voiceNotify, "appId", "a68e98684e5547ea874b00628e6ea28f");
        cJSON_AddStringToObject(voiceNotify, "to", number);
        cJSON_AddStringToObject(voiceNotify, "type", "1");
        cJSON_AddStringToObject(voiceNotify, "content", "469802");
        cJSON_AddStringToObject(voiceNotify, "toSerNum", "075512345678");
        cJSON_AddStringToObject(voiceNotify, "playTimes", "1");

        cJSON_AddItemToObject(root, "voiceNotify", voiceNotify);

        char *data = cJSON_PrintUnformatted(root);

        LOG_DEBUG("%s", timeString);
        LOG_DEBUG("%s", URL);
        LOG_DEBUG("%s", data);
        LOG_HEX(URL,256);
        LOG_HEX(data,150);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//print the error unexpected
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, getYunzxHeader(timeString));

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_Send_onPush);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            LOG_ERROR("call alarm send failed: %d:%s", res,curl_easy_strerror(res));
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





