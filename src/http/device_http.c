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

#define SIMCOM_URL "http://test.xiaoan110.com"
#define SIMCOM_HTTPPORT ":8082"
#define SIMCOM_URI "/v1/device"

#define MSG_MAX_LEN 1024

extern struct event_base *base;

typedef struct
{
    struct evhttp_request *req;
    struct evhttp_connection *connect;
}HTTP_CONNECTION;

static void http_wild2App(struct evhttp_request *req, void *arg)
{
    HTTP_CONNECTION *connection = arg;
	evhttp_connection_free(connection->connect);

    char post_data[MSG_MAX_LEN] = {0};
    evbuffer_copyout(req->input_buffer,post_data,MSG_MAX_LEN);
    LOG_INFO("get the response from simcom:%s", post_data);

    http_rspMsg(connection->req,post_data);
    return;
}

static void http_wild2Simcom(struct evhttp_request *req, char *url, char *data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);// parse url to uri, will be free at last
	int port = evhttp_uri_get_port(uri);// get the port from uri

	struct evhttp_connection *connect = evhttp_connection_base_new(base, NULL, evhttp_uri_get_host(uri), (port == -1 ? 8082 : port));

    HTTP_CONNECTION *http_connection = (HTTP_CONNECTION *)malloc(sizeof(HTTP_CONNECTION));

    http_connection->req = req;// store the req for reply to app, will be free in http_wild2App
    http_connection->connect = connect;// store the connect, will be free in http_wild2App

	struct evhttp_request *post = evhttp_request_new(http_wild2App, http_connection);// new request

    evbuffer_add(post->output_buffer, data, strlen(data));// post data
    evhttp_add_header(post->output_headers, "Content-Type", "application/json");
    evhttp_add_header(post->output_headers, "Host", evhttp_uri_get_host(uri));
    evhttp_make_request(connect, post, EVHTTP_REQ_POST, evhttp_uri_get_path(uri));// publish the request
    free(uri);

    //set a timer to cancel and free the connection and req if request can't get response

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

