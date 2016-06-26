//
// Created by jk on 16-6-25.
//

#ifndef ELECTROMBILE_PROTOCOL_H
#define ELECTROMBILE_PROTOCOL_H

#define START_FLAG_UDP (0xA5A5)
#define MAX_IMEI_LENGTH 15

typedef struct
{
    int timestamp;
    float longitude;
    float latitude;
    char speed;
    short course;
}__attribute__((__packed__)) GPS;

/*
 * UDP Message header definition
 */
typedef struct
{
    short signature;
    char imei[15];
    char cmd;
    short length;
}__attribute__((__packed__)) MSG_UDP_HEADER;

#define MSG_UDP_HEADER_LEN sizeof(MSG_UDP_HEADER)

/*
 * UDP packed GPS message structure
 */
typedef struct
{
    MSG_UDP_HEADER header;
    GPS gps[];
}__attribute__((__packed__)) MSG_GPS_PACK_UDP;


#endif //ELECTROMBILE_PROTOCOL_H
