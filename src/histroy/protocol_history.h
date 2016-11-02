/*
 * protocol_history.h
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */

#ifndef _MANAGER_HISTORY_H_
#define _MANAGER_HISTORY_H_

#define MANAGER_START_FLAG  (0xAA66)
#define MANAGER_MAX_IMEI_LENGTH (15)

#pragma pack(push, 1)

typedef struct
{
    int timestamp;
    float longitude;
    float latitude;
    char speed;
    short course;
}__attribute__((__packed__)) HISTORY_GPS;

typedef struct
{
    int num;
    HISTORY_GPS gps[];
}__attribute__((__packed__)) HISTORY_GPS_RSP;

typedef struct
{
    int starttime;
    float startlat;
    float startlon;
    int endtime;
    float endlat;
    float endlon;
    short miles;
}__attribute__((__packed__)) HISTORY_ITINERARY;

typedef struct
{
    int num;
    HISTORY_ITINERARY itinerary[];
}__attribute__((__packed__)) HISTORY_ITINERARY_RSP;

#pragma pack(pop)

#endif /* _MANAGER_PROTOCOL_H_ */

