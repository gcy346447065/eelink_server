#include "DB.h"
#include "logger.h"
#include "iserver.h"
#include "setting.h"
#include "UdpServerDataHandle.h"

#define PORT 8888

int main(int argc, char **argv)
{
    InitLog("../conf/gps_log.conf");

    int rc = setting_initail("../conf/eelink_server.ini");
    if (rc < 0)
    {
        LOG_ERROR() <<"eelink_server.ini failed: rc=" << rc;
    	return rc;
    }

    IUdpServer* pUdpServer = CreateUdpServerInstance();
    CUdpServerDataHandle udpServerDataHandle;
    pUdpServer->SetDataHandle(&udpServerDataHandle);

    LOG_INFO() << "gps server started";

    pUdpServer->Start(0, PORT);

    LOG_ERROR() << "gps server stoped";

    return 0;
}
