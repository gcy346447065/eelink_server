/*
 * protocol_manager.h
 *
 *  Created on: 2016/4/15
 *      Author: gcy
 */

#ifndef _MANAGER_PROTOCOL_H_
#define _MANAGER_PROTOCOL_H_

#define MANAGER_START_FLAG  (0xAA66)
#define MANAGER_MAX_IMEI_LENGTH (15)

enum
{
    MANAGER_CMD_LOGIN           =  1,
    MANAGER_CMD_IMEI_DATA       =  2,
    MANAGER_CMD_GET_LOG         =  3,
    MANAGER_CMD_GET_433         =  4,
    MANAGER_CMD_GET_GSM         =  5,
    MANAGER_CMD_GET_GPS         =  6,
    MANAGER_CMD_GET_SETTING     =  7,
    MANAGER_CMD_GET_BATTERY     =  8,
    MANAGER_CMD_REBOOT          =  9,
    MANAGER_CMD_UPGRADE         = 10,
    MANAGER_CMD_GET_AT          = 11,
    MANAGER_CMD_SET_SERVER      = 14,
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
}__attribute__((__packed__)) MANAGER_MSG_HEADER;

#define MANAGER_MSG_HEADER_LEN sizeof(MANAGER_MSG_HEADER)

/*
 * login message structure
 */
typedef MANAGER_MSG_HEADER MANAGER_MSG_LOGIN_REQ;

typedef MANAGER_MSG_HEADER MANAGER_MSG_LOGIN_RSP;

/*
 * imei data message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_IMEI_DATA_REQ;

typedef struct
{
    int timestamp;
    float longitude;
    float latitude;
    char speed;
    short course;
}__attribute__((__packed__)) MANAGER_GPS;

typedef struct
{
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
    char online_offline; // 1 for online; 2 for offline
    int version;
    MANAGER_GPS gps;
    char voltage;
    char gsm;
}__attribute__((__packed__)) MANAGER_IMEI_DATA;

typedef struct
{
    MANAGER_MSG_HEADER header;
    MANAGER_IMEI_DATA imei_data;
}__attribute__((__packed__)) MANAGER_MSG_IMEI_DATA_RSP;

typedef MANAGER_MSG_HEADER MANAGER_MSG_IMEI_DATA_ONCE_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_IMEI_DATA_ONCE_RSP;


/*
 * get daily message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_IMEI_DAILY_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_IMEI_DAILY_RSP;

/*
 * get log message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_LOG_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_LOG_RSP;

/*
 * get 433 message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_433_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_433_RSP;

/*
 * get GSM message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_GSM_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_GSM_RSP;

/*
 * get GPS message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_GPS_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_GPS_RSP;

/*
 * get setting message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_SETTING_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_SETTING_RSP;

/*
 * get battery message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_GET_BATTERY_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_GET_BATTERY_RSP;

/*
 * reboot message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_REBOOT_REQ;

/*
 * upgrade message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MANAGER_MSG_UPGRADE_REQ;

/*
 * server message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
    char server[];
}__attribute__((__packed__)) MANAGER_MSG_SETSERVER_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
}__attribute__((__packed__)) MANAGER_MSG_SETSERVER_RSP;

/*
 * AT message structure
 */
typedef struct
{
    MANAGER_MSG_HEADER header;
    char IMEI[MANAGER_MAX_IMEI_LENGTH];
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_AT_REQ;

typedef struct
{
    MANAGER_MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MANAGER_MSG_AT_RSP;



#pragma pack(pop)

#endif /* _MANAGER_PROTOCOL_H_ */
