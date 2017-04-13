//
// Created by lc on 16-6-25.
//

#ifndef ELECTROMBILE_MQTTTOP_H
#define ELECTROMBILE_MQTTTOP_H
#include <iostream>
#include <mosquittopp.h>

#include "protocol.h"
#include "setting.h"
#include "logger.h"

class myMosq : public mosqpp::mosquittopp
{
private:
    static int port;
    static int  keepalive;
    static string id;
    static string host;

    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);

private:
    myMosq() : mosquittopp(id.data()) {
        port = setting.mqtt_port;
        host = setting.mqtt_host;
        mosqpp::lib_init();
        reconnect_delay_set(10, 120, false);
        connect_async(host.data(), port, keepalive);
        loop_start();
    };

    ~myMosq() {
        loop_stop();
        mosqpp::lib_cleanup();
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

