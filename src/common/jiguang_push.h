/*
 * jiguang_push.h
 *
 *  Created on: May 10, 2016
 *      Author: gcy
 */

#ifndef COMMON_JIGUANG_PUSH_H_
#define COMMON_JIGUANG_PUSH_H_

#include "cJSON.h"

#define JIGUANG_APPKEY "b6b26e2547ad8e5f6018b225"

enum JIGUANG_CMD
{
    JIGUANG_CMD_ALARM             =  1,
    JIGUANG_CMD_AUTOLOCK_NOTIFY   =  2
};

int jiguang_push(char *imei, int yunba_cmd, int status);
size_t jiguang_onPush(void *contents, size_t size, size_t nmemb, void *userdata);

#endif /* COMMON_JIGUANG_PUSH_H_ */
