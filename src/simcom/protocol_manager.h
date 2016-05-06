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
    CMD_IMEI_DATA       =  2
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


#pragma pack(pop)

#endif /* _MANAGER_PROTOCOL_H_ */
