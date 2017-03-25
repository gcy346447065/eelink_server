/*
 * http.c
 *
 *  Created on: 20160203
 *      Author: jk
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

#include "log.h"
#include "msg_http.h"
#include "setting.h"


typedef struct
{
    struct evhttp_request *req;
    struct evhttp_connection *evcon;
}HTTP_CONNECTION;

static void http_wild2App(struct evhttp_request *req, void *arg)
{
    HTTP_CONNECTION *connection = arg;

    evhttp_connection_free(connection->evcon);

    if(NULL == req)
    {
        http_errorReply(connection->req, CODE_SIMCOM_OFF);
        return;
    }

    switch(req->response_code)
    {
        case HTTP_OK:
        case HTTP_OK+1:
            {
                char post_data[MAX_MSGHTTP_LEN] = {0};
                evbuffer_copyout(req->input_buffer,post_data,MAX_MSGHTTP_LEN);
                LOG_INFO("get the response from simcom:%s", post_data);
                http_postReply(connection->req,post_data);
                return;
            }

        default:
            {
                http_errorReply(connection->req, CODE_SIMCOM_OFF);
                return;
            }
            break;
    }

    return;
}

static void http_close_cb(struct evhttp_connection *evcon __attribute__((unused)), void *arg __attribute__((unused)))
{
    LOG_INFO("connection closed, if not get response http_server wild crash");
}

void http_sendData(struct event_base *base, struct evhttp_request *req, char *url, char *data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);// parse url to uri, will be free at last
	int port = evhttp_uri_get_port(uri);// get the port from uri

    // no need dns base
	struct evhttp_connection *evcon = evhttp_connection_base_new(base, NULL, evhttp_uri_get_host(uri), (port == -1 ? setting.simhttp_port: port));
    if (!evcon)
    {
        LOG_ERROR("couldn't generate connection");
        evhttp_uri_free(uri);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    HTTP_CONNECTION *http_connection = (HTTP_CONNECTION *)malloc(sizeof(HTTP_CONNECTION));

    http_connection->req = req;// store the req for reply to app, will be free in http_wild2App
    http_connection->evcon = evcon;// store the connect, will be free in http_wild2App

    //evhttp_connection_set_closecb(evcon, http_close_cb,NULL);// http_close_cb will first be called, useless

	struct evhttp_request *post = evhttp_request_new(http_wild2App, http_connection);// http_wild2App will be second called
	if(!post)
    {
        LOG_ERROR("couldn't generate request");
        evhttp_uri_free(uri);
        evhttp_connection_free(http_connection->evcon);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    evbuffer_add(post->output_buffer, data, strlen(data));// post data
    evhttp_add_header(post->output_headers, "Content-Type", "application/json");
    evhttp_add_header(post->output_headers, "Host", evhttp_uri_get_host(uri));

    int rc = evhttp_make_request(evcon, post, EVHTTP_REQ_POST, evhttp_uri_get_path(uri));// publish the post request
    if (rc == -1)
    {
        LOG_ERROR("couldn't make request");
        evhttp_connection_free(http_connection->evcon);
        http_errorReply(req, CODE_INTERNAL_ERROR);
        return;
    }

    evhttp_uri_free(uri);// free evhttp_uri_parse
    return;
}


void http_postReply(struct evhttp_request *req, char *data)
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

void http_okReply(struct evhttp_request *req)
{
    http_postReply(req, NULL);
    return;
}

void http_errorReply(struct evhttp_request *req, int errorType)
{
    char errorCode[32] = {0};
    snprintf(errorCode, 32, "{\"code\":%d}", errorType);
    http_postReply(req,errorCode);
    return;
}

