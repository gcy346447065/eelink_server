//
// Created by jk on 16-6-25.
//
#include <stdio.h>
#include <netinet/in.h>

#include "Message.h"
#include "protocol.h"
#include "DB.h"

using namespace std;

void Message::process()
{
    GPS *gps = reinterpret_cast<GPS*> (data);

    string IMEI(imei, imei + IMEI_LENGTH);

    for(int i = 0; i < ntohs(length)/sizeof(GPS); i++)
    {
        DB::instance().addGPS(IMEI, gps + i);
    }
}
