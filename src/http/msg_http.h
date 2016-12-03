/*
 * msg_http.h
 *
 *  Created on: 2016/11/25
 *      Author: lc
 */
#ifndef SRC_MSG_HTTP_H_
#define SRC_MSG_HTTP_H_

#include <evhttp.h>

void http_rspMsg(struct evhttp_request *req, char *data);
void http_errorMsg(struct evhttp_request *req);
void http_okMsg(struct evhttp_request *req);


#define MYHTTPD_SIGNATURE   "http v1"
#define SECONDS_PER_DAY (86400)

#endif

