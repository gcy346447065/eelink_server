/*
 * yunba_push.h
 *
 *  Created on: May 14, 2015
 *      Author: jk
 */

#ifndef SRC_YUNBA_PUSH_H_
#define SRC_YUNBA_PUSH_H_

#include "cJSON.h"

#define YUNBA_APPKEY "554ab0957e353f5814e12100"

int yunba_connect();
void yunba_disconnect();
void yunba_publish(char* topicName, void* payload, int payloadLen);

#endif /* SRC_YUNBA_PUSH_H_ */
