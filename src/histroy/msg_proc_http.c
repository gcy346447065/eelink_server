/*
 * msg_proc_http.c
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#include <stdio.h>

#include "msg_proc_http.h"
#include "protocol_history.h"

#include "log.h"
#include "malloc.h"
#include "db.h"

void history_getGPS(const char *imeiName, int starttime, int endtime)
{
    HISTORY_GPS_RSP *gps = (HISTORY_GPS_RSP *)db_getGPS(imeiName, starttime, endtime);
    printf("num:%d\r\n", gps->num);
    for(int i = 0; i < gps->num;i++)
    {
        printf("%d\r\n", gps->gps[i].timestamp);
        printf("%f\r\n", gps->gps[i].latitude);
        printf("%f\r\n", gps->gps[i].longitude);
        printf("%f\r\n", gps->gps[i].speed);
        printf("%f\r\n", gps->gps[i].course);
    }

    free(gps);
}


