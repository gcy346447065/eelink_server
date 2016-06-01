/*
 * jiguang_push.c
 *
 *  Created on: Jue 1, 2016
 *      Author: lc
 */

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "sms_message.h"
#include "log.h"

size_t SMS_Message_onPush(void *contents, size_t size, size_t nmemb, void *userdata)
{
    userdata = userdata;

    char* rsp = malloc(size * nmemb + 1);
    memcpy(rsp, contents, size * nmemb);
    rsp[size * nmemb] = 0;

    cJSON* json = cJSON_Parse(rsp);

    if (!json)
    {
        LOG_ERROR("error sms message response: %s", rsp);
    }
    else
    {
        LOG_DEBUG("get sms message response: %s", rsp);
    }

    cJSON_Delete(json);
    free(rsp);

    return size * nmemb;
}

int SMS_Alarm_Send(char *imei, char *number)
{
    int ret = 0;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        char data[128] = {0};
        cJSON *vars = cJSON_CreateObject();
        cJSON_AddStringToObject(vars, "imei", imei);
        snprintf(data,128,\
            "appid=10885&to=%s&project=bDuSy2&signature=0be45370fa575ecbda1101c3b70296ba&vars=%s",\
            number,\
            cJSON_PrintUnformatted(vars));
        cJSON_Delete(vars);

        LOG_DEBUG("%s", data);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.submail.cn/message/xsend");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SMS_Message_onPush);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            LOG_ERROR("sms message send failed: %s", curl_easy_strerror(res));
            ret = -2;
        }

        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_ERROR("message send curl_easy_init failed");
        ret = -1;
    }

    LOG_DEBUG("sms message send: number(%s), imei(%s)", number, imei);

    return ret;
}

int SMS_Message_Send(char *content, char *number)//TODO: need a project id
{
    int ret = 0;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        char data[128] = {0};
        cJSON *vars = cJSON_CreateObject();
        cJSON_AddStringToObject(vars, "content", content);
        snprintf(data,128,\
            "appid=10885&to=%s&project=bDuSy2&signature=0be45370fa575ecbda1101c3b70296ba&vars=%s",\
            number,\
            cJSON_PrintUnformatted(vars));
        cJSON_Delete(vars);

        LOG_DEBUG("%s", data);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.submail.cn/message/xsend");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SMS_Message_onPush);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            LOG_ERROR("sms message send failed: %s", curl_easy_strerror(res));
            ret = -2;
        }

        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_ERROR("sms message send curl_easy_init failed");
        ret = -1;
    }

    LOG_DEBUG("sms message send: number(%s), content(%s)", number, content);

    return ret;
}


