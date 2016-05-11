/*
 * jiguang_push.c
 *
 *  Created on: May 11, 2016
 *      Author: jk
 */

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "jiguang_push.h"
#include "cJSON.h"
#include "log.h"

static struct curl_slist *getJiguangHeader(void)
{
    static struct curl_slist *headerlist = NULL;
    if(!headerlist)
    {
        headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
    }

    return headerlist;
}

int jiguang_push(char *imei, int jiguang_cmd, int status)
{
    int ret = 0;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "platform", "all");
        cJSON_AddStringToObject(root, "audience", "all");

        cJSON *notification = cJSON_CreateObject();
        cJSON_AddStringToObject(notification, "alert", "test for jiguang push");
        cJSON_AddItemToObject(root, "notification", notification);

        char *data = cJSON_PrintUnformatted(root);
        LOG_DEBUG("%s", data);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.jpush.cn/v3/push");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, getJiguangHeader());
        curl_easy_setopt(curl, CURLOPT_USERNAME, "b6b26e2547ad8e5f6018b225:ce9800560f464ea8b815407f");
        
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data)); /* size of the POST data */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, jiguang_onPush);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            LOG_ERROR("jiguang_push failed: %s", curl_easy_strerror(res));
            ret = -2;
        }

        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_ERROR("jiguang_push curl_easy_init failed");
        ret = -1;
    }

    LOG_DEBUG("push to jiguang: imei(%s), jiguang_cmd(%d), status(%d)", imei, jiguang_cmd, status);

    return ret;
}

size_t jiguang_onPush(void *contents, size_t size, size_t nmemb, void *userdata)
{
    userdata = userdata;

    char* rsp = malloc(size * nmemb + 1);
    memcpy(rsp, contents, size * nmemb);
    rsp[size * nmemb] = 0;

    cJSON* json = cJSON_Parse(rsp);

    if (!json)
    {
        LOG_ERROR("error jiguang_onPush response: %s", rsp);
    }
    else
    {
        LOG_DEBUG("get jiguang_onPush response: %s", rsp);
    }

    cJSON_Delete(json);
    free(rsp);

    return size * nmemb;
}
