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
    //TODO: to be fixed
    printf("signature:%4x\r\n",ntohs(signature));
    printf("imei:%s\r\n",imei);
    printf("cmd:%d\r\n",cmd);
    printf("length:%d\r\n",ntohs(length));

    string IMEI(imei, imei + IMEI_LENGTH);
    GPS *temp = NULL;
    GPS *gps = reinterpret_cast<GPS*> (data);
    for(int i = 0; i < ntohs(length)/sizeof(GPS); i++)
    {
        temp = (GPS *)gps + i;
        DB::instance().addGPS(imei, temp);
    }
}
