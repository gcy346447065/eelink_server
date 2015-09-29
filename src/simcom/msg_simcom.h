/*
 * msg_simcon.h
 *
 *  Created on: 2015/6/29
 *      Author: jk
 */

#ifndef SRC_MSG_SIMCOM_H_
#define SRC_MSG_SIMCOM_H_

#include <stdio.h>

#include "protocol.h"

MSG_HEADER* alloc_simcom_msg(char cmd, size_t length);
MSG_HEADER* alloc_simcom_rspMsg(const MSG_HEADER* pMsg);

void free_simcom_msg(MSG_HEADER* msg);

const char *getIMEI(const char *imei);

#endif /* SRC_MSG_SIMCOM_H_ */
