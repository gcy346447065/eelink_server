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
    int httpd_timeout = 5;         // in seconds

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc >= 2)
    {
    	char* port = argv[1];
    	int num = atoi(port);
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

    struct event_base *base = event_base_new();
	if (!base)
    {
		LOG_ERROR("event_base_new failed...");
		return 1;
	}

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

	struct evhttp *httpd = evhttp_new(base);
	if (!httpd) {
		LOG_ERROR("couldn't create evhttp. Exiting.");
		return 1;
	}

    if (evhttp_bind_socket(httpd, httpd_listen, http_port) != 0)
    {
        LOG_ERROR("bind socket failed at port:%d", http_port);
        return 1;
    }

    evhttp_set_timeout(httpd, httpd_timeout);
    evhttp_set_gencb(httpd, httpd_handler, base);


    LOG_INFO("start http_server: %d", http_port);

    event_base_dispatch(base);

    LOG_INFO("stop http_server: %d", http_port);

    evhttp_free(httpd);

    event_base_free(base);
    db_destruct();
    zlog_fini();
    return 0;
}

