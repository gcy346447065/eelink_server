//
// Created by jk on 15-10-30.
//

#ifndef ELECTROMBILE_INTER_MSG_H
#define ELECTROMBILE_INTER_MSG_H
enum
{
    CMD_SYNC_NEW_DID,
    CMD_SYNC_NEW_GPS,
    CMD_SYNC_NEW_ITINERARY,
    CMD_SYNC_NEW_SIM_INFO
};

#define TAG_CMD "CMD"
#define TAG_TIMESTAMP   "TIMESTAMP"
#define TAG_IMEI "IMEI"
#define TAG_LAT "LAT"
#define TAG_LNG "LNG"
#define TAG_SPEED "SPEED"
#define TAG_COURSE "COURSE"
#define TAG_DID "DID"

#define TAG_CCID "CCID"
#define TAG_IMSI "IMSI"

#define TAG_START "START"
#define TAG_END "END"
#define TAG_MILES "MILES"

#endif //ELECTROMBILE_INTER_MSG_H
