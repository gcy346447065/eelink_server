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
#include "cJSON.h"
#include "msg_http.h"
#include "device_http.h"
static void http_replyDevice(struct evhttp_request *req)
{
    LOG_INFO("%s",req->uri);

    http_errorMsg(req);
    return;
}

void http_deviceHandler(struct evhttp_request *req)
{
}


