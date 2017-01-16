/*
 * device_http.c
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "log.h"
#include "port.h"
#include "msg_http.h"
#include "cJSON.h"
#include "device_http.h"
#include "redis.h"

#define DEVICE_URI "/v1/device"

static void http_deviceTransfer(struct evhttp_request *req, struct event_base *base)
{
    char url[64] = {0};
    char IMEI[15 + 1] = {0};
    char post_data[MAX_MSGHTTP_LEN] = {0};

    evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);
    LOG_INFO("get the request from app:%s", post_data);

    cJSON *root = cJSON_Parse(post_data);
    if(!root)
    {
        LOG_ERROR("content is not json type");
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    cJSON *imei = cJSON_GetObjectItem(root, "imei");
    if(!imei)
    {
        LOG_ERROR("no imei");
        cJSON_Delete(root);
        http_errorReply(req, CODE_ERROR_CONTENT);
        return;
    }

    strncpy(IMEI, imei->valuestring, 15);
    cJSON_Delete(root);

    int rc = redis_getDeviceServer(IMEI, url);
    if(rc)
    {
        LOG_INFO("device(%s) offline", IMEI);
        http_errorReply(req, CODE_DEVICE_OFF);
    }
    else
    {
        strcat(url, DEVICE_URI);
        LOG_INFO("get url: %s", url);
        http_sendData(base,req, url, post_data);
    }
}

void http_deviceHandler(struct evhttp_request *req, struct event_base *base)
{
    switch(req->type)
    {
        case EVHTTP_REQ_POST:
            {
                http_deviceTransfer(req, base);
                return;
            }
            break;

        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_DELETE:
            break;
        default:
            break;
    }
    http_errorReply(req, CODE_URL_ERR);
    return;
}

