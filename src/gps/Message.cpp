//
// Created by jk on 16-6-25.
//

#include "Message.h"
#include "protocol.h"
#include "DB.h"

void Message::process() {
    //TODO: to be fixed
    GPS* gps = reinterpret_cast<GPS*> (data);
    DB::instance().addGPS(imei, gps);
}