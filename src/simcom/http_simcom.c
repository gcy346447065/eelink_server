/*
 * http_simcom.c
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#include <string.h>

#include "http_simcom.h"
#include "session.h"
#include "log.h"
#include "cJSON.h"
#include "object.h"

#define MSG_MAX_LEN 1024

static void simcom_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSG_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);

    LOG_INFO("get the request from http:%s", post_data);

    cJSON *json = cJSON_Parse(post_data);
    if(!json)
    {
        simcom_replyHttp(req, NULL);
        return;
    }

    char *imei = cJSON_GetObjectItem(json, "imei")->valuestring;
    if(!imei)
    {
        simcom_replyHttp(req, NULL);
        return;
    }

    OBJECT *obj = obj_get(imei);
    if(!obj)
    {
        simcom_replyHttp(req, NULL);
        return;
    }

    SESSION *session = obj->session;
    if(!session)
    {
        simcom_replyHttp(req, NULL);
        return;
    }
    session->req = req;

    return;
}

void simcom_http_handler(struct evhttp_request *req, void *arg __attribute__((unused)))
{
    if(strstr(req->uri, "/v1/device"))
    {
        simcom_deviceHandler(req);
    }
    return;
}

void simcom_replyHttp(struct evhttp_request *req, const char *data)
{
    evhttp_add_header(req->output_headers, "Server", "simcom v1");
    evhttp_add_header(req->output_headers, "Content-Type", "application/json");
    evhttp_add_header(req->output_headers, "Connection", "close");
    struct evbuffer *buf = evbuffer_new();
    if(data)
    {
        evbuffer_add_printf(buf, "%s", data);
    }
    else
    {
        evbuffer_add_printf(buf, "%s", "{\"code\":101}");
    }
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    return;
}

