//
// Created by jk on 16-6-25.
//

#ifndef ELECTROMBILE_MESSAGE_H
#define ELECTROMBILE_MESSAGE_H


class Message {
public:
    static const int IMEI_LENGTH = 15;
    static const unsigned short START_FLAG_UDP = 0xA5A5;

private:
    short signature;
    char imei[IMEI_LENGTH];
    char cmd;
    short length;

    char data[];

private:
    void handle_cmd_gps();

public:
    void process();

};


#endif //ELECTROMBILE_MESSAGE_H
