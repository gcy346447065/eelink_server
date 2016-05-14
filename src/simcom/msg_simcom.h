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

void free_simcom_msg(void* msg);

char get_msg_cmd(void *msg);

void *alloc_simcomWildMsg(const char* data, size_t length);
void *alloc_simcomDefendReq(int token, char operator);
void *alloc_simcomSeekReq(int token, char operator);
void *alloc_simcomAutolockSetReq(int token, char onOff);
void *alloc_simcomAutoPeriodSetReq(int token, char period);
void *alloc_simcomAutoPeriodGetReq(int token);
void *alloc_simcomAutolockGetReq(int token);
void *alloc_simcomUpgradeStartReq(int version, int size);
void *alloc_simcomUpgradeDataReq(int offset, char *data, int length);
void *alloc_simcomUpgradeEndReq(int checksum, int size);

void *alloc_simcomManagerReq(int cmd);

#endif /* SRC_MSG_SIMCOM_H_ */
