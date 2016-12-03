/*
 * http_simcom.h
 *
 *  Created on: 2016/12/03
 *      Author: lc
 */
#ifndef MSG_HTTP_SIMCOM_H_
#define MSG_HTTP_SIMCOM_H_
#include <evhttp.h>

typedef struct
{
    struct evhttp_request *req;
	OBJECT* obj;
	HTTP_RSP_PROC* pfn;
}HTTP_CONNECTION;

#endif /* MSG_HTTP_SIMCOM_H_ */

