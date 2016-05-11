/*
 * jiguang_push.c
 *
 *  Created on: May 11, 2016
 *      Author: jk
 */

#include <string.h>
#include <stdlib.h>

#include "jiguang_push.h"
#include "cJSON.h"
#include "curl.h"

#define JIGUANG_URL "https://api.jpush.cn/v3/push"

static int jiguang_push(char *imei, int yunba_cmd, int status)
{
    int ret = 0;
    CURL *curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, JIGUANG_URL);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); /* pass in a pointer to the data - libcurl will not copy */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); /* size of the POST data */

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

    return ret;
}

