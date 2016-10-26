//
// Created by jk on 16-6-25.
//
#include <netinet/in.h>
#include <glog/logging.h>

#include "Message.h"
#include "protocol.h"
#include "DB.h"

using namespace std;

void Message::process()
{
    if (signature != START_FLAG_UDP)
    {
        LOG(ERROR) << "message signature not valid";
        return;
    }

    switch (cmd)
    {
        case CMD_UDP_GPS:
            LOG(INFO) << "handle gps message";
            handle_cmd_gps();
            break;
        default:
            break;
    }

}

void Message::handle_cmd_gps()
{
    string IMEI(imei, imei + IMEI_LENGTH);

    GPS *gps = reinterpret_cast<GPS*> (data);


    for(int i = 0; i < ntohs(length)/sizeof(GPS); i++)
    {
        DB::instance().addGPS(IMEI, gps + i);
    }
}
