/*
 * http_simcom.h
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#ifndef MSG_HTTP_SIMCOM_H_
#define MSG_HTTP_SIMCOM_H_
#include <evhttp.h>

void simcom_http_handler(struct evhttp_request *req, void *arg __attribute__((unused)));
void simcom_replyHttp(struct evhttp_request *req, const char *data);
void simcom_errorHttp(struct evhttp_request *req, int errorType);

enum
{
    CODE_INTERNAL_ERR       = 200,
    CODE_IMEI_NOT_FOUND     = 201,
    CODE_NO_CONTENT         = 202,
    CODE_RANGE_TOO_LARGE    = 203,
    CODE_URL_ERR            = 204,
    CODE_ERROR_CONTENT      = 205,
    CODE_DEVICE_NOT_RSP     = 206,
    CODE_DEVICE_OFFLINE     = 207,
    CODE_DEVICE_NO_RESPONSE = 208,

};

#endif /* MSG_HTTP_SIMCOM_H_ */

