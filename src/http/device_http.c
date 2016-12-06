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
#include "cJSON.h"
#include "msg_http.h"
#include "device_http.h"

#define SIMCOM_URL "http://test.xiaoan110.com"
#define SIMCOM_HTTPPORT ":8082"
#define SIMCOM_URI "/v1/device"

#define MSG_MAX_LEN 256

extern struct event_base *base;

typedef struct
{
    struct evhttp_request *req;
    struct evhttp_connection *evcon;
}HTTP_CONNECTION;

static void http_wild2App(struct evhttp_request *req, void *arg)
{
    HTTP_CONNECTION *connection = arg;
    evhttp_connection_free(connection->evcon);
    char post_data[MSG_MAX_LEN] = {0};
    switch(req->response_code)
    {
        case HTTP_OK:
        case HTTP_OK+1:
            {
                evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);
                LOG_INFO("get the response from simcom:%s", post_data);
                break;
            }

        default:
            {
                http_errorMsg(connection->req, CODE_SIMCOM_OFFLINE);
                return;
            }
            break;
    }

    http_rspMsg(connection->req,post_data);
    return;
}

static void http_close_cb(struct evhttp_connection *evcon __attribute__((unused)), void *arg __attribute__((unused)))
{
    LOG_INFO("connection closed, if not get response http_server wild crash");
}

static void http_wild2Simcom(struct evhttp_request *req, char *url, char *data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);// parse url to uri, will be free at last
	int port = evhttp_uri_get_port(uri);// get the port from uri

    // no need dns base
	struct evhttp_connection *evcon = evhttp_connection_base_new(base, NULL, evhttp_uri_get_host(uri), (port == -1 ? PORT_SIMCOMHTTP : port));
    if (!evcon)
    {
        LOG_ERROR("couldn't generate connection");
        evhttp_uri_free(uri);
        http_errorMsg(req, CODE_INTERNAL_ERR);
        return;
    }

    HTTP_CONNECTION *http_connection = (HTTP_CONNECTION *)malloc(sizeof(HTTP_CONNECTION));

    http_connection->req = req;// store the req for reply to app, will be free in http_wild2App
    http_connection->evcon = evcon;// store the connect, will be free in http_wild2App

    evhttp_connection_set_closecb(evcon, http_close_cb,NULL);// http_close_cb will first be called, useless

	struct evhttp_request *post = evhttp_request_new(http_wild2App, http_connection);// http_wild2App will be second called
	if(!post)
    {
        LOG_ERROR("couldn't generate request");
        evhttp_uri_free(uri);
        evhttp_connection_free(http_connection->evcon);
        http_errorMsg(req, CODE_INTERNAL_ERR);
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
        http_errorMsg(req, CODE_INTERNAL_ERR);
        return;
    }

    evhttp_uri_free(uri);// free evhttp_uri_parse
    return;
}

void http_deviceHandler(struct evhttp_request *req)
{
    char post_data[MSG_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);
    LOG_INFO("get the request from app:%s", post_data);
    http_wild2Simcom(req, SIMCOM_URL SIMCOM_HTTPPORT SIMCOM_URI, post_data);
    return;
}

