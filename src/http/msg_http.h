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
void http_okMsg(struct evhttp_request *req);
void http_errorMsg(struct evhttp_request *req, int errorType);

enum
{
    CODE_INTERNAL_ERR       = 100,
    CODE_IMEI_NOT_FOUND     = 101,
    CODE_NO_CONTENT         = 102,
    CODE_RANGE_TOO_LARGE    = 103,
    CODE_URL_ERR            = 104,
    CODE_ERROR_CONTENT      = 105,
    CODE_SIMCOM_NO_RSP      = 106,
    CODE_SIMCOM_OFFLINE     = 107,
};
#endif

