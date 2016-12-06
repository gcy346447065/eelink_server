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
#include "http.h"
#include "cJSON.h"
#include "device_http.h"

#define SIMCOM_URL "http://test.xiaoan110.com"
#define SIMCOM_HTTPPORT ":8082"
#define SIMCOM_URI "/v1/device"

 extern struct event_base *base;

void http_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSGHTTP_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSGHTTP_MAX_LEN);
    LOG_INFO("get the request from app:%s", post_data);
    http_wild2Simcom(base,req, SIMCOM_URL SIMCOM_HTTPPORT SIMCOM_URI, post_data);
    return;
}

