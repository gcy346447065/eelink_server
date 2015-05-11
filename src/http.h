/*
 * http.h
 *
 *  Created on: 2015��4��21��
 *      Author: jk
 */

#ifndef SRC_HTTP_H_
#define SRC_HTTP_H_

#include "cb_ctx_mc.h"

typedef int HTTP_RSP_PROC(int response_code, const char* msg, CB_CTX* ctx);

typedef struct
{
	CB_CTX *ctx;
	HTTP_RSP_PROC* pfn;
}HTTP_SESSION;

void init_session(HTTP_SESSION* session, CB_CTX* ctx, HTTP_RSP_PROC* pfn);

void *http_get(HTTP_SESSION* session, const char *url);

void *http_post(HTTP_SESSION* session, const char *url,  const char* data);

#endif /* SRC_HTTP_H_ */
