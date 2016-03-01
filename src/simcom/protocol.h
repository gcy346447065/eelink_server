/*
 * protocol.h
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define START_FLAG (0xAA55)
#define MAX_IMEI_LENGTH 15
#define MAX_CCID_LENGTH 20
#define MAX_CELL_NUM 7
#define TEL_NO_LENGTH 11

enum
{
    CMD_WILD            =  0,
    CMD_LOGIN           =  1,
    CMD_PING            =  2,
    CMD_GPS             =  3,
    CMD_CELL            =  4,
    CMD_ALARM           =  5,
    CMD_SMS             =  6,
    CMD_433             =  7,
    CMD_DEFEND          =  8,
    CMD_SEEK            =  9,
    CMD_LOCATE          = 10,
    CMD_SET_SERVER      = 11,
    CMD_SET_TIMER       = 12,
    CMD_SET_AUTOSWITCH  = 13,
    CMD_GET_AUTOSWITCH  = 14,
    CMD_SET_PERIOD      = 15,
    CMD_GET_PERIOD      = 16,
    CMD_ITINERARY       = 17,
    CMD_BATTERY         = 18,
    CMD_DEFEND_ON       = 19,
    CMD_DEFEND_OFF      = 20,
    CMD_DEFEND_GET      = 21,
    CMD_DEFEND_NOTIFY   = 22
};

enum
{
    MSG_SUCCESS = 0,
};

#pragma pack(push, 1)
/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char cmd;
    short seq;
    short length;
}__attribute__((__packed__)) MSG_HEADER;

#define MSG_HEADER_LEN sizeof(MSG_HEADER)

/*
 * Login message structure
 */
typedef struct
{
    MSG_HEADER header;
    char Version;
    char IMEI[MAX_IMEI_LENGTH];
    char CCID[MAX_CCID_LENGTH];
}__attribute__((__packed__)) MSG_LOGIN_REQ;

typedef MSG_HEADER MSG_LOGIN_RSP;

/*
 * ping message structure
 */
typedef struct
{
    MSG_HEADER header;
    short statue;   //TODO: to define the status bits
}__attribute__((__packed__)) MSG_PING_REQ;

typedef MSG_HEADER MSG_PING_RSP;

/*
 * GPS message structure
 */
typedef struct
{
    float longitude;
    float latitude;
    float altitude;
    float speed;
    float course;
}__attribute__((__packed__)) GPS;

typedef struct
{
    MSG_HEADER header;
    GPS gps;
}__attribute__((__packed__)) MSG_GPS;

/*
 * CGI message structure
 */
typedef struct
{
    short lac;       //local area code
    short cellid;    //cell id
    short rxl;       //receive level
}__attribute__((__packed__)) CELL;

typedef struct
{
    short mcc;      //mobile country code
    short mnc;      //mobile network code
    char  cellNo;   //cell count
    //CELL cell[];
}__attribute__((__packed__)) CGI;       //Cell Global Identifier

typedef struct
{
    MSG_HEADER header;
    CGI cgi;
}__attribute__((__packed__)) MSG_CGI;

/*
 * alarm message structure
 */
enum ALARM_TYPE
{
    ALARM_FENCE_OUT,
    ALARM_FENCE_IN,
    ALARM_VIBRATE,
};

typedef struct
{
    MSG_HEADER header;
    char alarmType;
}__attribute__((__packed__)) MSG_ALARM_REQ;

typedef MSG_HEADER MSG_ALARM_RSP;

/*
 * SMS message structure
 */
typedef struct
{
    MSG_HEADER header;
    char telphone[TEL_NO_LENGTH + 1];
    char smsLen;
    char sms[];
}__attribute__((__packed__)) MSG_SMS_REQ;

typedef MSG_SMS_REQ MSG_SMS_RSP;

/*
 * 433 message structure
 * the message has no response
 */
typedef struct
{
    MSG_HEADER header;
    int intensity;
}__attribute__((__packed__)) MSG_433_REQ;

/*
 * defend message structure
 */
enum DEFEND_TYPE
{
    DEFEND_ON   = 0x01,
    DEFEND_OFF  = 0x02,
    DEFEND_GET  = 0x03,
};

typedef struct
{
    MSG_HEADER header;
    int token;
    char operator;     // refer to DEFEND_TYPE
}__attribute__((__packed__)) MSG_DEFEND_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_DEFEND_RSP;

/*
 * seek message structure
 */
enum SEEK_TYPE
{
    SEEK_OFF    = 0x01,
    SEEK_ON     = 0x02,
};

typedef struct
{
    MSG_HEADER header;
    int token;
    char operator;     //refer to SEEK_TYPE
}__attribute__((__packed__)) MSG_SEEK_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_SEEK_RSP;

/*
 * server set_ip/domain message structure
 * this message has no response
 */
typedef struct
{
    MSG_HEADER header;
    int port;
    char server[];
}__attribute__((__packed__)) MSG_SERVER;

/*
 * GPS set_time message structure
 */
typedef struct
{
    MSG_HEADER header;
    int timer;
}__attribute__((__packed__)) MSG_GPSTIMER_REQ;

typedef struct
{
    MSG_HEADER header;
    int result;
}__attribute__((__packed__)) MSG_GPSTIMER_RSP;

/*
 * autolock set message structure
 */
enum AUTOLOCK_TYPE
{
    AUTOLOCK_OFF,
    AUTOLOCK_ON
};

typedef struct
{
    MSG_HEADER header;
    int token;
    char onOff;     //refer to AUTOLOCK_TYPE
}__attribute__((__packed__)) MSG_AUTOLOCK_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTOLOCK_SET_RSP;

/*
 * autolock get message structure
 */
typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTOLOCK_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_AUTOLOCK_GET_RSP;

/*
 * autoperiod set message structure
 */
typedef struct
{
    MSG_HEADER header;
    int token;
    char period;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTOPERIOD_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;
}__attribute__((__packed__)) MSG_AUTOPERIOD_SET_RSP;

/*
 * autoperiod get message structure
 */
typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTOPERIOD_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    char result;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTOPERIOD_GET_RSP;

/*
 * mileage upload message structure
 */
typedef struct
{
    MSG_HEADER header;
    int start;
    int end;
    int mileage;    //unit: meters
}__attribute__((__packed__)) MSG_MILEAGE_REQ;

/*
 * battery message structure
 */
typedef struct
{
    MSG_HEADER header;
    char percent;
    char miles;    //unit: kilometers
}__attribute__((__packed__)) MSG_BATTERY_RSP;

/*
 * defend switch message structure
 */
typedef struct
{
    MSG_HEADER header;
    char result;
}__attribute__((__packed__)) MSG_DEFEND_ON_RSP;

typedef struct
{
    MSG_HEADER header;
    char result;
}__attribute__((__packed__)) MSG_DEFEND_OFF_RSP;

typedef struct
{
    MSG_HEADER header;
    char status;             //0: OFF,1: ON
}__attribute__((__packed__)) MSG_DEFEND_GET_RSP;

typedef struct
{
    MSG_HEADER header;
    char status;             //0: OFF,1: ON
}__attribute__((__packed__)) MSG_DEFEND_NOTIFY_RSP;

#pragma pack(pop)

#endif /* _PROTOCOL_H_ */
