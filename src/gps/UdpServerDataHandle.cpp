//
// Created by jk on 16-6-17.
//
#include <new>

#include "UdpServerDataHandle.h"
#include "Message.h"

void CUdpServerDataHandle::OnDataReceived(unsigned int IP, unsigned short Port, const char *pData, int nLen)
{
    if (nLen < sizeof(Message))
    {
        return;
    }
    void* pMsg = const_cast<void*>(static_cast<const void *>(pData));

    Message* msg = new (pMsg) Message;
    msg->process();
    msg->~Message();
}
