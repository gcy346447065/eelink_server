
#include <glog/logging.h>


#include "iserver.h"
#include "UdpServerDataHandle.h"

#define PORT 8888

int main(int argc, char **argv)
{

    google::InitGoogleLogging(argv[0]);


    IUdpServer* pUdpServer = CreateUdpServerInstance();
    CUdpServerDataHandle udpServerDataHandle;
    pUdpServer->SetDataHandle(&udpServerDataHandle);

    LOG(INFO) << "gps server started";

    pUdpServer->Start(0, PORT);

    return 0;
}
