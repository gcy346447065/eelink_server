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

extern struct event_base *base;

typedef struct
{
    struct evhttp_request *req;
    struct evhttp_connection *connect;
}HTTP_CONNECTION;

void http_requset_post_cb(struct evhttp_request *req, void *arg)
{
    HTTP_CONNECTION *connection = arg;
	evhttp_connection_free(connection->connect);

    char post_data[128] = {0};
    evbuffer_copyout(req->input_buffer,post_data,128);
    LOG_INFO("%s", post_data);

    http_rspMsg(connection->req,post_data);
    return;
}

static void http_wild2Simcom(struct evhttp_request *req, char *url, char *data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	int port = evhttp_uri_get_port(uri);
	struct evhttp_connection *connect = evhttp_connection_base_new(base, NULL, evhttp_uri_get_host(uri), (port == -1 ? 8082 : port));

    HTTP_CONNECTION *http_connection = (HTTP_CONNECTION *)malloc(sizeof(HTTP_CONNECTION));
    http_connection->req = req;
    http_connection->connect = connect;

	struct evhttp_request *post = evhttp_request_new(http_requset_post_cb, http_connection);

    evbuffer_add(post->output_buffer, data, strlen(data));
    evhttp_add_header(post->output_headers, "Content-Type", "application/json");
    evhttp_add_header(post->output_headers, "Host", evhttp_uri_get_host(uri));

    evhttp_make_request(connect, post, EVHTTP_REQ_POST, evhttp_uri_get_path(uri));
    free(uri);
    return;
}

void http_deviceHandler(struct evhttp_request *req)
{
    char post_data[128] = {0};
    evbuffer_copyout(req->input_buffer,post_data,128);
    LOG_INFO("%s",post_data);
    http_wild2Simcom(req, SIMCOM_URL SIMCOM_HTTPPORT SIMCOM_URI, post_data);
    return;
}


