/*
 * protocol_manager.h
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */

#ifndef _MANAGER_PROTOCOL_H_
#define _MANAGER_PROTOCOL_H_

#define START_FLAG  (0xAA66)
#define MAX_IMEI_LENGTH (15)

enum
{
    CMD_LOGIN           =  1,
    CMD_IMEI_DATA       =  2,
    CMD_GET_LOG         =  3,
    CMD_GET_SETTING     =  4,
    CMD_GET_BATTERY     =  5,
    CMD_GET_GSM         =  6,
    CMD_GET_433         =  7,
    CMD_GET_GPS         =  8,
    CMD_REBOOT          =  9,
    CMD_UPGRADE         = 10,
    CMD_GET_VERSION     = 11
};

#pragma pack(push, 1)
/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char cmd;
    char seq;
    short length;
}__attribute__((__packed__)) MSG_HEADER;

#define MSG_HEADER_LEN sizeof(MSG_HEADER)

/*
 * login message structure
 */
typedef MSG_HEADER MSG_LOGIN_REQ;

typedef MSG_HEADER MSG_LOGIN_RSP;

/*
 * imei data message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_IMEI_DATA_REQ;

typedef struct
{
    int timestamp;
    float longitude;
    float latitude;
    char speed;
    short course;
}__attribute__((__packed__)) GPS;

typedef struct
{
    char IMEI[MAX_IMEI_LENGTH];
    char online_offline; //1 for online; 2 for offline
    GPS  gps;
}__attribute__((__packed__)) IMEI_DATA;

typedef struct
{
    MSG_HEADER header;
    IMEI_DATA imei_data;
}__attribute__((__packed__)) MSG_IMEI_DATA_RSP;

/*
 * get log message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_LOG_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_LOG_RSP;

/*
 * get setting message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_SETTING_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_SETTING_RSP;

/*
 * get battery message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_BATTERY_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_BATTERY_RSP;

/*
 * get GSM message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_GSM_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_GSM_RSP;

/*
 * get 433 message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_433_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_433_RSP;

/*
 * get GPS message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_GPS_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_GPS_RSP;

/*
 * reboot message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_REBOOT_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_REBOOT_RSP;

/*
 * upgrade message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_UPGRADE_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_UPGRADE_RSP;

/*
 * get version message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_GET_VERSION_REQ;

typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_GET_VERSION_RSP;

#pragma pack(pop)

#endif /* _MANAGER_PROTOCOL_H_ */
