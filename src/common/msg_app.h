/*
 * msg_app.h
 *
 *  Created on: May 12, 2015
 *      Author: jk
 */

#ifndef SRC_MSG_APP_H_
#define SRC_MSG_APP_H_

enum CMD
{
    APP_CMD_WILD        = 0,
    APP_CMD_FENCE_ON    = 1,
    APP_CMD_FENCE_OFF   = 2,
    APP_CMD_FENCE_GET   = 3,
    APP_CMD_SEEK_ON     = 4,
    APP_CMD_SEEK_OFF    = 5,
    APP_CMD_LOCATION    = 6,
    APP_CMD_AUTOLOCK_ON,
    APP_CMD_AUTOLOCK_OFF,
    APP_CMD_AUTOPERIOD_SET,
    APP_CMD_AUTOPERIOD_GET,

};

enum RESULT
{
    ERR_SUCCESS     =   0,
    ERR_INTERNAL    = 100,
    ERR_WAITING     = 101,
    ERR_OFFLINE     = 102,
};

enum ALARM
{
    ALM_FENCE_OUT       = 0,
    ALM_FENCE_IN        = 1,
    ALM_VIBRATE         = 2,
    ALM_FENCE_IN_TK115  = 0x83,
    ALM_FENCE_OUT_TK115 = 0x84,
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

void app_sendCmdRsp2App(int cmd, int result, const char *strIMEI);

void app_sendFenceGetRspMsg2App(int cmd, int result, int state, void *session);
void app_sendGpsMsg2App(void* session);
void app_send433Msg2App(int timestamp, int intensity, void * session);
void app_sendAutolockMsg2App(int timestamp, int lock, void * session);
void app_sendAlarmMsg2App(unsigned char type, const char *msg, void *session);

#endif /* SRC_MSG_APP_H_ */
