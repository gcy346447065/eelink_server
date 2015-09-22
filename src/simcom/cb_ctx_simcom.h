/*
 * cb_ctx_simcom.h
 *
 *  Created on: Jul 10, 2015
 *      Author: jk
 */

#ifndef SRC_CB_CTX_SIMCOM_H_
#define SRC_CB_CTX_SIMCOM_H_


#include "env.h"

typedef void (*SIMCOM_MSG_SEND)(struct bufferevent* bev, const void* buf, size_t n);

typedef struct
{
	ENVIRONMENT* env;
	struct event_base* base;
	struct bufferevent* bev;

	void* obj;
	SIMCOM_MSG_SEND pSendMsg;
}SIMCOM_CTX;

void ctx_table_initial();
void ctx_table_destruct();

int ctx_add(SIMCOM_CTX *);
int ctx_del(SIMCOM_CTX *);
SIMCOM_CTX* ctx_get(const char* imei);

#endif /* SRC_CB_CTX_SIMCOM_H_ */
