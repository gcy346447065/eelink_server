/*
 * redis.h
 *
 *  Created on: 2017/1/16
 *      Author: lc
 */
#ifndef __USER_HIREDIS_H__
#define __USER_HIREDIS_H__

#include <hiredis/hiredis.h>

#define MAX_HOSTNAMEWITHPORT_LEN (64)

int redis_initial(void);
int redis_AddDevice(const char *imei);
int redis_DelDevice(const char *imei);
int redis_getDeviceServer(const char *imei, char *hostNamewithPort);
int redis_getSelfSimcomIp(char * hostSimcom);

#endif/*__USER_HIREDIS_H__*/


