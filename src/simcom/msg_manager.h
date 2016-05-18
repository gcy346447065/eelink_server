/*
 * msg_manager.h
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */

#ifndef SRC_MSG_MANAGER_H_
#define SRC_MSG_MANAGER_H_

#include <stdio.h>
#include "protocol_manager.h"

MANAGER_MSG_HEADER* alloc_manager_msg(char cmd, size_t length);
MANAGER_MSG_HEADER* alloc_manager_rspMsg(const MANAGER_MSG_HEADER *pMsg);

void *alloc_managerSimcomRsp(int cmd, int data_length);

void free_manager_msg(void *msg);
char get_manager_msg_cmd(void *msg);

#endif /* SRC_MSG_MANAGER_H_ */