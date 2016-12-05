/*
 * package_http.h
 *
 *  Created on: 2016/12/02
 *      Author: lc
 */
#ifndef SRC_PACKAGE_HTTP_H_
#define SRC_PACKAGE_HTTP_H_

#include <evhttp.h>

void http_replyVersion(struct evhttp_request *req);
void http_replyPackage(struct evhttp_request *req);


#endif/* SRC_PACKAGE_HTTP_H_ */

