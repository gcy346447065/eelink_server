/*
 * http_simcom.c
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#include <string.h>

#include "http_simcom.h"
#include "session.h"

static void simcom_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSG_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);
    LOG_INFO("get the request from http:%s", post_data);
    return;
}

void simcom_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    if(strstr(req->uri, "/v1/device"))
    {
        simcom_deviceHandler(req);
    }
    return;
}

