/*
 * http.h
 *
 *  Created on: 20160203
 *      Author: jk
 */

#ifndef SRC_HTTP_H_
#define SRC_HTTP_H_

#include "object.h"

typedef int HTTP_RSP_PROC(int response_code, const char* msg, OBJECT* obj);

typedef struct
{
    struct event_base* base;
	OBJECT* obj;
	HTTP_RSP_PROC* pfn;
}HTTP_CONNECTION;

void init_session(HTTP_CONNECTION *session, struct event_base* base, OBJECT *object, HTTP_RSP_PROC *pfn);

void *http_get(HTTP_CONNECTION *session, const char *url);

void *http_post(HTTP_CONNECTION *session, const char *url,  const char *data);

#endif /* SRC_HTTP_H_ */
