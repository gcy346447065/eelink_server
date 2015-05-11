/*
 * http.c
 *
 *  Created on: 2015��4��21��
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
#include "http.h"

#ifdef WITH_CATEGORY

#define LOG_DEBUG(...) \
	zlog(cat[MOD_HTTP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_HTTP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_HTTP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_HTTP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_HTTP], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)

#endif

// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"
// (use for files: picture, mp3, tar-file etc.)
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"


void init_session(HTTP_SESSION* session, CB_CTX* ctx, HTTP_RSP_PROC* pfn)
{
	session->ctx = ctx;
	session->pfn = pfn;
}

static inline void print_request_head_info(struct evkeyvalq *header)
{
    struct evkeyval *first_node = header->tqh_first;
    while (first_node) {
        LOG_DEBUG("key:%s  value:%s", first_node->key, first_node->value);
        first_node = first_node->next.tqe_next;
    }
}

void http_requset_post_cb(struct evhttp_request *req, void *arg)
{
	HTTP_SESSION *session = arg;
    switch(req->response_code)
    {
        case HTTP_OK:
        case HTTP_OK + 1:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);

            print_request_head_info(req->output_headers);

            LOG_DEBUG("len:%zu  body size:%zu", len, req->body_size);
            char *body = malloc(len+1);
            memcpy(body, evbuffer_pullup(buf, -1), len);
            body[len] = '\0';

            LOG_DEBUG("HTML BODY:%s", body);

            session->pfn(req->response_code, body, session->ctx);
            free(body);

//            event_base_loopexit(http_req_post->base, 0);
            break;
        }

        case HTTP_BADREQUEST:
        	LOG_ERROR("http post responde error:status_code %d %s",
        			req->response_code,
        			req->response_code_line);
        	session->pfn(req->response_code, NULL, session->ctx);
        	return;

        case HTTP_MOVEPERM:
            LOG_ERROR("%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
//            const char *new_location = evhttp_find_header(req->input_headers, "Location");
//            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
//            evhttp_uri_free(ctx->uri);
//            http_req_post->uri = new_uri;
//            start_url_request((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);
            return;
        }

        default:
//            event_base_loopexit(http_req_post->base, 0);
            return;
    }

    free(session);
}

void http_requset_get_cb(struct evhttp_request *req, void *arg)
{
	HTTP_SESSION *session = arg;

    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            LOG_DEBUG("print the head info:");
            print_request_head_info(req->output_headers);

            LOG_DEBUG("len:%zu  body size:%zu", len, req->body_size);
            char *body = malloc(len+1);
            memcpy(body, evbuffer_pullup(buf, -1), len);
            body[len] = '\0';
            LOG_DEBUG("print the body:");
            LOG_DEBUG("HTML BODY:%s", body);

            session->pfn(req->response_code, body, session->ctx);
            free(body);

//            event_base_loopexit(base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            LOG_ERROR("%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
//            const char *new_location = evhttp_find_header(req->input_headers, "Location");
//            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
//            evhttp_uri_free(http_req_get->uri);
//            http_req_get->uri = new_uri;
//            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }

        default:
//            event_base_loopexit(http_req_get->base, 0);
            return;
    }

    free(session);
}

static inline void print_uri_parts_info(const struct evhttp_uri * http_uri)
{
    LOG_DEBUG("scheme:%s", evhttp_uri_get_scheme(http_uri));
    LOG_DEBUG("host:%s", evhttp_uri_get_host(http_uri));
    LOG_DEBUG("path:%s", evhttp_uri_get_path(http_uri));
    LOG_DEBUG("port:%d", evhttp_uri_get_port(http_uri));
    LOG_DEBUG("query:%s", evhttp_uri_get_query(http_uri));
    LOG_DEBUG("userinfo:%s", evhttp_uri_get_userinfo(http_uri));
    LOG_DEBUG("fragment:%s", evhttp_uri_get_fragment(http_uri));
}

void *http_get(HTTP_SESSION* session, const char *url)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	print_uri_parts_info(uri);

	int port = evhttp_uri_get_port(uri);
	struct evhttp_connection *cn = evhttp_connection_base_new(session->ctx->base,
            NULL,
            evhttp_uri_get_host(uri),
            (port == -1 ? 80 : port));

	//TODO: set the timeout
	//evhttp_connection_set_timeout(cn, 1000);

    /* does request will be released by evhttp connection???   */
	struct evhttp_request *req = evhttp_request_new(http_requset_get_cb, session);

    const char *query = evhttp_uri_get_query(uri);
    const char *path = evhttp_uri_get_path(uri);
    char *path_query = NULL;

    if (query)
    {
        size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
        if (len > 1) {
            path_query = calloc(len, sizeof(char));
            sprintf(path_query, "%s?%s", path, query);
        }
    }
    else
    {
    	path_query = path;
    }

    evhttp_make_request(cn, req, EVHTTP_REQ_GET, path_query ? path_query: "/");
    /** Set the header properties */
    evhttp_add_header(req->output_headers, "Host", evhttp_uri_get_host(uri));

    return NULL;	//TODO: change the return type
}

void *http_post(HTTP_SESSION* session, const char *url,  const char* data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	print_uri_parts_info(uri);

	char* post_data = strdup(data);

	int port = evhttp_uri_get_port(uri);
	struct evhttp_connection *cn = evhttp_connection_base_new(session->ctx->base,
            NULL,
            evhttp_uri_get_host(uri),
            (port == -1 ? 80 : port));

    /* does request will be released by evhttp connection???   */
	struct evhttp_request *req = evhttp_request_new(http_requset_post_cb, session);

    const char *path = evhttp_uri_get_path(uri);
    evhttp_make_request(cn, req, EVHTTP_REQ_POST, path ? path : "/");

    /** Set the post data */
    evbuffer_add(req->output_buffer, post_data, strlen(post_data));
    evhttp_add_header(req->output_headers, "Content-Type", HTTP_CONTENT_TYPE_URL_ENCODED);
    evhttp_add_header(req->output_headers, "Host", evhttp_uri_get_host(uri));

    return NULL;	//TODO: change the return type
}
