/*
 * fsm.h
 *
 *  Created on: 2015��4��20��
 *      Author: jk
 */

#ifndef SRC_FSM_H_
#define SRC_FSM_H_

typedef enum
{
	STS_INITIAL,
	STS_WAIT_DID,
	STS_WAIT_M2MINFO,
	STS_WAIT_OTA,
	STS_WAIT_FIRMWARE,
	STS_LOGINING,
	STS_RUNNING,
	STS_MAX
}STATUS;

typedef enum
{
	EVT_SIGN_IN,
	EVT_GOT_DID,
	EVT_GOT_M2M,
	EVT_FIRMWARE,
	EVT_LOGIN,
	EVT_PING,
	EVT_MQTT,
	EVT_MAX
}EVENT;

int fsm_run(EVENT, void* ctx);

int start_fsm(void* ctx);

#endif /* SRC_FSM_H_ */
