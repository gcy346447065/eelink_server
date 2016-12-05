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

#endif /* MSG_HTTP_SIMCOM_H_ */

