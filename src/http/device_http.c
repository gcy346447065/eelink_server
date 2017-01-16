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

void http_deviceHandler(struct evhttp_request *req, struct event_base *base)
{
    switch(req->type)
    {
        case EVHTTP_REQ_POST:
            {
                char url[64] = {0};
                char hostname[64] = {0};
                char post_data[MAX_MSGHTTP_LEN] = {0};
                snprintf(url, 64, "http://%s:%d%s", HOST_SIMCOM, PORT_SIMCOMHTTP, DEVICE_URI);
                evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);
                int rc = redis_getDeviceServer("865067022405313", hostname);
                if(rc)
                {
                    LOG_INFO("DEVICE 865067022405313 OFF");
                    http_errorReply(req, CODE_DEVICE_OFF);
                }
                LOG_INFO("get server: %s", hostname);
                LOG_INFO("get the request from app:%s", post_data);
                http_sendData(base,req, url, post_data);
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

