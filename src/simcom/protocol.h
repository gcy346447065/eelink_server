/*
 * protocol.h
 *
 *  Created on: 2015年6月29日
 *      Author: jk
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define START_FLAG (0xAA55)
#define IMEI_LENGTH 16
#define MAX_CELL_NUM 7
#define TEL_NO_LENGTH 11


enum
{
    CMD_WILD        = 0x00,
    CMD_LOGIN       = 0x01,
    CMD_GPS         = 0x02,
    CMD_CELL        = 0x03,
    CMD_PING        = 0x04,
    CMD_ALARM       = 0x05,
    CMD_SMS         = 0x06,
    CMD_433         = 0x07,
    CMD_DEFEND      = 0x08,
    CMD_SEEK        = 0x09,
    CMD_LOCATION    = 0x0a,
    CMD_SERVER      = 0x0b,
    CMD_TIMER       = 0x0c,
    CMD_AUTODEFEND_SWITCH_SET,
    CMD_AUTODEFEND_SWITCH_GET,
    CMD_AUTODEFEND_PERIOD_SET,
    CMD_AUTODEFEND_PERIOD_GET,
    CMD_AUTODEFEND_NOTIFY,

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
    unsigned short seq;
    unsigned short length;
}__attribute__((__packed__)) MSG_HEADER;

#define MSG_HEADER_LEN sizeof(MSG_HEADER)

/*
 * Login message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[IMEI_LENGTH];
}__attribute__((__packed__)) MSG_LOGIN_REQ;

typedef MSG_HEADER MSG_LOGIN_RSP;

/*
 * GPS structure
 */
typedef struct
{
    float longitude;
    float latitude;
}__attribute__((__packed__)) GPS;

/*
 * GPS message structure
 */
typedef struct
{
    MSG_HEADER header;
    GPS gps;
}__attribute__((__packed__)) MSG_GPS;

/*
 * CELL structure
 */
typedef struct
{
   short lac;       //local area code
   short cellid;    //cell id
   short rxl;       //receive level
}__attribute__((__packed__)) CELL;

typedef struct
{
    short mcc;  //mobile country code
    short mnc;  //mobile network code
    char  cellNo;// cell count
//    CELL cell[];
}__attribute__((__packed__)) CGI;       //Cell Global Identifier

/*
 * CGI message structure
 */
 typedef struct
 {
     MSG_HEADER header;
     CGI cgi;
 }__attribute__((__packed__)) MSG_CGI;

/*
 * heartbeat message structure
 */
typedef struct
{
    MSG_HEADER header;
    short statue;   //TODO: to define the status bits
}__attribute__((__packed__)) MSG_PING_REQ;

typedef MSG_HEADER MSG_PING_RSP;

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
    unsigned char alarmType;
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
 * seek message structure
 * the message has no response
 */
typedef struct
{
    MSG_HEADER header;
    int intensity;
}__attribute__((__packed__)) MSG_433;

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
    unsigned char operator;     // refer to DEFEND_TYPE
}__attribute__((__packed__)) MSG_DEFEND_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_DEFEND_RSP;

/*
 * switch on the seek mode
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
    unsigned char operator;     //refer to SEEK_TYPE
}__attribute__((__packed__)) MSG_SEEK_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_SEEK_RSP;

typedef MSG_HEADER MSG_LOCATION;




enum AUTODEFEND_SWITCH
{
    AUTO_DEFEND_OFF,
    AUTO_DEFEND_ON
};

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char onOff; //refer to AUTODEFEND_SWITCH
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_SET_RSP;

typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_SWITCH_GET_RSP;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char period;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_SET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char result;
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_SET_RSP;


typedef struct
{
    MSG_HEADER header;
    int token;
}__attribute__((__packed__)) MSG_AUTODEFEND_PERIOD_GET_REQ;

typedef struct
{
    MSG_HEADER header;
    int token;
    unsigned char period;   //time unit: minutes
}__attribute__((__packed__)) MSG_AUTODEFEND_PEROID_GET_RSP;


typedef struct
{
    MSG_HEADER header;
    int lock;
}__attribute__((__packed__)) MSG_AUTOLOCK;

#pragma pack(pop)

#endif /* _PROTOCOL_H_ */
