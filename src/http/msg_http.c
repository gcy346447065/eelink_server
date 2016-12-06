/*
 * msg_http.c
 *
 *  Created on: 2016/11/25
 *      Author: lc
 */

#include "msg_http.h"

#define MYHTTPD_SIGNATURE   "http v1"

void http_rspMsg(struct evhttp_request *req, char *data)
{
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "application/json");
    evhttp_add_header(req->output_headers, "Connection", "close");
    struct evbuffer *buf = evbuffer_new();
    if(data)
    {
        evbuffer_add_printf(buf, "%s", data);
    }
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    return;
}

void http_okMsg(struct evhttp_request *req)
{
    http_rspMsg(req, NULL);
    return;
}

void http_errorMsg(struct evhttp_request *req, int errorType)
{
    char errorCode[32] = {0};
    snprintf(errorCode, 32, "{\"code\":%d}", errorType);
    http_rspMsg(req,errorCode);
    return;
}

