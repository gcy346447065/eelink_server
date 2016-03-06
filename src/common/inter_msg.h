//
// Created by jk on 15-10-30.
//

#ifndef ELECTROMBILE_INTER_MSG_H
#define ELECTROMBILE_INTER_MSG_H
enum
{
    CMD_SYNC_NEW_DID,
    CMD_SYNC_NEW_GPS,
};

#define TAG_CMD "CMD"
#define TAG_TIMESTAMP   "TIMESTAMP"
#define TAG_IMEI "IMEI"
#define TAG_LAT "LAT"
#define TAG_LNG "LNG"
#define TAG_ALTITUDE "ALTITUDE"
#define TAG_SPEED "SPEED"
#define TAG_COURSE "COURSE"

#endif //ELECTROMBILE_INTER_MSG_H
