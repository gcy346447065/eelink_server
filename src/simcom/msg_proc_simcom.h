/*
 * msg_proc_simcom.h
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */

#ifndef SRC_MSG_PROC_SIMCOM_H_
#define SRC_MSG_PROC_SIMCOM_H_

#include <stdio.h>
#include "session.h"

int handle_simcom_msg(const char* m, size_t msgLen, void* arg);
int simcom_msg_send(void *msg, size_t len, SESSION *session);


#endif /* SRC_MSG_PROC_SIMCOM_H_ */
