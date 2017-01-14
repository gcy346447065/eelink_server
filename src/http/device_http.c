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

#define SIMCOM_URL "http://localhost"
#define SIMCOM_HTTPPORT ":8082"
#define SIMCOM_URI "/v1/device"

void http_deviceHandler(struct evhttp_request *req, struct event_base *base)
{
    switch(req->type)
    {
        case EVHTTP_REQ_POST:
            {
                char post_data[MAX_MSGHTTP_LEN] = {0};
                evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);
                LOG_INFO("get the request from app:%s", post_data);
                http_sendData(base,req, SIMCOM_URL SIMCOM_HTTPPORT SIMCOM_URI, post_data);
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

