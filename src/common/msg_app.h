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
};

enum RESULT
{
    ERR_SUCCESS     =   0,
    ERR_INTERNAL    = 100,
    ERR_WAITING     = 101,
    ERR_OFFLINE     = 102,
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


#endif /* SRC_MSG_APP_H_ */
