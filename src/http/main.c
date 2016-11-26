#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>    //for struct evkeyvalq
#include <event.h>
#include <evhttp.h>
#include <signal.h>

#include "db.h"
#include "log.h"
#include "port.h"
#include "macro.h"
#include "msg_proc_http.h"

static void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGINT:
            LOG_WARN("http server being stopped");
            event_loopbreak();
            break;

        default:
            LOG_INFO("unkown signal happened: %d", sig);
            break;
    }
}

int main(int argc, char *argv[])
{
    int http_port = PORT_HTTP;
    char *httpd_listen = "0.0.0.0"; // locaol adress
    int httpd_timeout = 30;         // in seconds

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc >= 2)
    {
    	char* strPort = argv[1];
    	int num = atoi(strPort);
    	if (num)
    	{
    		http_port = num;
    	}
    }

    int rc = log_init("../conf/http_log.conf");
    if (rc)
    {
        LOG_ERROR("log initial failed: rc=%d", rc);
    	return rc;
    }

    rc = db_initial();
    if(rc)
    {
        LOG_FATAL("connect to mysql failed");
        return -1;
    }

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    event_init();

    struct evhttp *httpd;
    httpd = evhttp_start(httpd_listen, http_port);
    evhttp_set_timeout(httpd, httpd_timeout);

    evhttp_set_gencb(httpd, httpd_handler, NULL);

    LOG_INFO("start http_server...");
    event_dispatch();

    db_destruct();

    zlog_fini();

    evhttp_free(httpd);
    return 0;
}

