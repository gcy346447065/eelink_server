//
// Created by jk on 16-6-17.
//

#ifndef ELECTROMBILE_UDPSERVERDATAHANDLE_H
#define ELECTROMBILE_UDPSERVERDATAHANDLE_H


#include "iserver.h"

class CUdpServerDataHandle: public IUdpServerDataHandle
{
public:
    virtual ~CUdpServerDataHandle(){}

    virtual void  OnDataReceived(unsigned int IP, unsigned short Port, const char * pData, int nLen);

};

#endif //ELECTROMBILE_UDPSERVERDATAHANDLE_H
