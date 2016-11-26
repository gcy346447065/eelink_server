/*
 * msg_proc_http.h
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#ifndef MSG_PROC_HTTP_H_
#define MSG_PROC_HTTP_H_
#include <evhttp.h>

void httpd_handler(struct evhttp_request *req, void *arg __attribute__((unused)));


#endif /* MSG_PROC_HTTP_H_ */

