//
// Created by jk on 16-6-25.
//

#ifndef ELECTROMBILE_PROTOCOL_H
#define ELECTROMBILE_PROTOCOL_H

typedef struct
{
    int timestamp;
    float longitude;
    float latitude;
    char speed;
    short course;
}__attribute__((__packed__)) GPS;

#endif //ELECTROMBILE_PROTOCOL_H
