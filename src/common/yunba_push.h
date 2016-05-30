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

enum YUNBA_CMD
{
    YUNBA_CMD_ALARM             =  1,
    YUNBA_CMD_AUTOLOCK_NOTIFY   =  2
};

int yunba_connect();
void yunba_disconnect();
void yunba_publish(char *imei, int yunba_cmd, int status);
void yunba_publish_old(char* topicName, char* payload, int payloadLen);

#endif /* SRC_YUNBA_PUSH_H_ */
