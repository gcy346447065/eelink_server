/*
 * itinerary_object.h
 *
 *  Created on: 2017/03/28
 *      Author: lc
 */

#ifndef SRC_ITINERARY_OBJECT_
#define SRC_ITINERARY_OBJECT_
#include "macro.h"

typedef struct
{
    char IMEI[IMEI_LENGTH + 1];

    int timestamp;
    float lat;
    float lon;
    char speed;
    short course;

    char isStarted;
    int starttime;
    float startlat;
    float startlon;
    int itineray;

    char timecount;

} ITINERARY_OBJECT;

typedef int (*ITINERARY_PROC)(const char* tableName, int starttime, float startlat, float startlon, int endtime, float endlat, float endlon, short itinerary);

void itineraryObj_table_initial();
void itineraryObj_table_destruct();

ITINERARY_OBJECT *itineraryObj_new();
void itineraryObj_add(ITINERARY_OBJECT *obj);
void itineraryObj_del(ITINERARY_OBJECT *obj);
ITINERARY_OBJECT *itineraryObj_get(const char *IMEI);

#endif /* SRC_ITINERARY_OBJECT_ */

