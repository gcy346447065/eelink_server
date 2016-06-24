#include <stdio.h>
#include <stdlib.h>

#include "iserver.h"
#include "UdpServerDataHandle.h"

#define PORT 8888

int main(int argc, char **argv)
{

    IUdpServer* pUdpServer = CreateUdpServerInstance();
    CUdpServerDataHandle udpServerDataHandle;
    pUdpServer->SetDataHandle(&udpServerDataHandle);

    pUdpServer->Start(0, PORT);

    return 0;
}
