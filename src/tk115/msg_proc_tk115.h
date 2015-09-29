/*
 * msg_proc_mc.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_PROC_TK115_H_
#define SRC_MSG_PROC_TK115_H_

#include "session.h"

int tk115_login(const void *msg, SESSION *ctx);
int tk115_gps(const void *msg, SESSION *ctx);
int tk115_ping(const void *msg, SESSION *ctx);
int tk115_alarm(const void *msg, SESSION *ctx);
int tk115_status(const void *msg, SESSION *ctx);
int tk115_sms(const void *msg, SESSION *ctx);
int tk115_operator(const void *msg, SESSION *ctx);
int tk115_data(const void *msg, SESSION *ctx);

int msg_send(void *msg, size_t len, SESSION *ctx);

#endif /* SRC_MSG_PROC_TK115_H_ */
