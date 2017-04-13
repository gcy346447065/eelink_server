//
// Created by jk on 16-6-25.
//
#include <netinet/in.h>

#include "DB.h"
#include "logger.h"
#include "Message.h"
#include "mqtt_top.h"
#include "protocol.h"

using namespace std;

void Message::process()
{
    if (ntohs(signature) != START_FLAG_UDP)
    {
        LOG_ERROR() << "message signature not valid";
        return;
    }

    switch (cmd)
    {
        case CMD_UDP_GPS:
            LOG_INFO() << "handle gps message";
            handle_cmd_gps();
            break;
        default:
            break;
    }
}

void Message::handle_cmd_gps()
{
    int i = 0;
    string IMEI(imei, imei + IMEI_LENGTH);
    GPS *gps = reinterpret_cast<GPS*> (data);

    for(i = 0; i < ntohs(length)/sizeof(GPS); i++)
    {
        DB::instance().addGPS(IMEI, gps + i);
    }

    myMosq::instance().send_message(IMEI.data(), gps + (i - 1));
}
