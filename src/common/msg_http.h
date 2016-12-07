/*
 * http.h
 *
 *  Created on: 2016/12/06
 *      Author: lc
 */

#ifndef SRC_HTTP_H_
#define SRC_HTTP_H_

#include <evhttp.h>

void http_okReply(struct evhttp_request *req);
void http_postReply(struct evhttp_request *req, char *data);
void http_errorReply(struct evhttp_request *req, int errorType);
void http_sendData(struct event_base *base, struct evhttp_request *req, char *url, char *data);

enum
{
    CODE_INTERNAL_ERROR     = 100,
    CODE_IMEI_NOT_FOUND     = 101,
    CODE_NO_CONTENT         = 102,
    CODE_RANGE_TOO_LARGE    = 103,
    CODE_URL_ERR            = 104,
    CODE_ERROR_CONTENT      = 105,
    CODE_SIMCOM_NO_RSP      = 106,
    CODE_SIMCOM_OFF         = 107,
    CODE_DEVICE_NOT_RSP     = 108,
    CODE_DEVICE_OFF         = 109,
    CODE_DEVICE_NO_RSP      = 120,
};

#define MAX_MSGHTTP_LEN 256
#define MAX_ERRCODE_LEN 32

#endif /* SRC_HTTP_H_ */
