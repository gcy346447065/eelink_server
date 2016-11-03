/*
 * msg_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_APP_H_
#define SRC_MSG_APP_H_

#include "object.h"

enum CMD
{
    APP_CMD_WILD                =  0,
    APP_CMD_FENCE_ON            =  1,
    APP_CMD_FENCE_OFF           =  2,
    APP_CMD_FENCE_GET           =  3,
    APP_CMD_SEEK_ON             =  4,
    APP_CMD_SEEK_OFF            =  5,
    APP_CMD_LOCATION            =  6,
    APP_CMD_AUTOLOCK_ON         =  7,
    APP_CMD_AUTOLOCK_OFF        =  8,
    APP_CMD_AUTOPERIOD_SET      =  9,
    APP_CMD_AUTOPERIOD_GET      = 10,
    APP_CMD_AUTOLOCK_GET        = 11,
    APP_CMD_BATTERY             = 12,
    APP_CMD_STATUS_GET          = 13,
    APP_CMD_GPS_SWITCH          = 14,
    APP_CMD_SET_BATTERY_TYPE    = 15,
};

enum CODE
{
    CODE_SUCCESS            =   0,
    CODE_INTERNAL_ERR       = 100,
    CODE_WAITING            = 101,
    CODE_DEVICE_OFFLINE     = 102,
    CODE_BATTERY_LEARNING   = 103
};

enum ALARM
{
    ALM_FENCE_OUT       = 0,
    ALM_FENCE_IN        = 1,
    ALM_VIBRATE         = 2,
    ALM_FENCE_IN_TK115  = 0x83,
    ALM_FENCE_OUT_TK115 = 0x84
};

enum NOTIFY
{
    NOTIFY_AUTOLOCK     = 1,
    NOTIFY_STATUS       = 2,
    NOTIFY_BATTERY      = 3
};

//Message definition
typedef struct
{
    short header;
    short cmd;
    unsigned short length;
    unsigned short seq;
    char data[];
}__attribute__((__packed__)) APP_MSG;

typedef struct
{
    short header;
    int timestamp;
    float lat;
    float lon;
    short course;
    char speed;
    char isGPS;
}__attribute__((__packed__)) GPS_MSG;

typedef struct
{
    short header;
    int intensity;
}__attribute__((__packed__)) F33_MSG;//433

void app_sendCmdRsp2App(int cmd, int code, const char *strIMEI);
void app_sendFenceGetRsp2App(int cmd, int code, int state, void *session);
void app_sendLocationRsp2App(int code, OBJECT *obj);
void app_sendAutoPeriodGetRsp2App(int cmd, int code, int period, void *session);
void app_sendAutoLockGetRsp2App(int cmd, int code, int state, void *session);
void app_sendBatteryRsp2App(int cmd, int code, int percent, int miles, void *session);
void app_sendStatusGetRsp2App(int cmd, int code, OBJECT *obj, char autolock, char autoperiod, char percent, char miles, char status);

void app_sendGpsMsg2App(void* session);
void app_send433Msg2App(int timestamp, int intensity, void * session);
void app_sendAlarmMsg2App(int type, const char *msg, void *session);
void app_sendDebugMsg2App(const char *msg, size_t length, void *session);
void app_sendNotifyMsg2App(int notify, int timestamp, int lock_status, void *session);

#endif /* SRC_MSG_APP_H_ */
