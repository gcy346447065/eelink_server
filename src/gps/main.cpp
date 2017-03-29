#include "logger.h"
#include "iserver.h"
#include "UdpServerDataHandle.h"

#define PORT 8888

int main(int argc, char **argv)
{
    InitLog("../conf/gps_log.conf");

    IUdpServer* pUdpServer = CreateUdpServerInstance();
    CUdpServerDataHandle udpServerDataHandle;
    pUdpServer->SetDataHandle(&udpServerDataHandle);

    LOG_INFO << "gps server started";

    pUdpServer->Start(0, PORT);

    LOG_ERROR << "gps server stoped";

    return 0;
}
