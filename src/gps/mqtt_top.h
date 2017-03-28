//
// Created by lc on 16-6-25.
//

#ifndef ELECTROMBILE_MQTTTOP_H
#define ELECTROMBILE_MQTTTOP_H
#include <iostream>
#include <mosquittopp.h>

#include "protocol.h"

class myMosq : public mosqpp::mosquittopp
{
private:
    static int port;
    static int keepalive;
    static const char *id;
    static const char *host;
    static const char *topic;

    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);

private:
    myMosq() : mosquittopp(id) {
        mosqpp::lib_init();     // Mandatory initialization for mosquitto library
        connect_async(host, port, keepalive);   // non blocking connection to broker request
        loop_start();           // Start thread managing connection / publish / subscribe
    };

    ~myMosq() {
        loop_stop();            // Kill the thread
        mosqpp::lib_cleanup();  // Mosquitto library cleanup
    }

public:
    static myMosq& instance() {
        static myMosq instance;
        return instance;
    }

    myMosq(myMosq const&)           = delete;
    void operator=(myMosq const&)   = delete;

public:
    bool send_message(const char *imei, GPS *gps);
};

#endif //ELECTROMBILE_MQTTTOP_H

