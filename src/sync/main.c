#include <event2/event.h>
#include <curl/curl.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <event2/listener.h>

#include "log.h"
#include "version.h"
#include "server_sync.h"
#include "leancloud_req.h"
#include "session.h"
#include "setting.h"
#include "env.h"
#include "db.h"
#include "timer.h"
#include "objectID_leancloud.h"

struct event_base *base = NULL;

void ResaveUnpostedImei_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    LOG_INFO("one-day timer for ResaveUnpostedImei_cb");

    db_ResaveOBJUnpostedImei_cb(arg); //leancloud_saveDid

    return;
}

static void sig_usr(int signo)
{
	if (signo == SIGINT)
	{
		printf("oops! catch CTRL+C!!!\n");
		event_base_loopbreak(base);
	}

	if (signo == SIGTERM)
	{
		printf("oops! being killed!!!\n");
		event_base_loopbreak(base);
	}

    return;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int rc = log_init("../conf/sync_log.conf");
    if (rc)
    {
        printf("log initial failed: rc=%d", rc);
    	return rc;
    }

    rc = setting_initail("../conf/eelink_server.ini");
    if (rc < 0)
    {
        LOG_ERROR("eelink_server.ini failed: rc=%d", rc);
    	return rc;
    }
    int port = setting.sync_port;

    LOG_INFO("Sync Server %s, with event %s, curl %s\n",
    		VERSION_STR, LIBEVENT_VERSION, curl_version());

    base = event_base_new();
    if (!base)
    {
        LOG_ERROR("Can't make new event base");
        return 1; /*XXXerr*/
    }

    if (signal(SIGINT, sig_usr) == SIG_ERR)
    {
        LOG_ERROR("Can't catch SIGINT");
    }

    if (signal(SIGTERM, sig_usr) == SIG_ERR)
    {
    	LOG_ERROR("Can't catch SIGTERM");
    }

    rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK)
    {
    	LOG_FATAL("curl lib initial failed:%d", rc);
    }

    rc = db_initial();
    if(rc)
    {
        LOG_FATAL("connect to mysql failed");
        return -1;
    }

    rc = objectID_table_initial();
    if(rc)
    {
        LOG_FATAL("objectID_table_initial failed");
        return -1;
    }

    struct evconnlistener *listener = server_sync(base, port);
    if (listener)
    {
        LOG_INFO("start sync server successfully at port:%d", port);
    }
    else
    {
        LOG_FATAL("start sync server failed at port:%d", port);
        return 2;
    }

    //start a one-day timer to resave multiple unsaved DIDs
    struct timeval one_day = { 86400, 0 };
    (void)timer_newLoop(base, &one_day, ResaveUnpostedImei_cb, leancloud_saveDid);

    env_initial();

    //start the event loop
    LOG_INFO("start the event loop");
    event_base_dispatch(base);

    env_cleanup();

    //sk_free(SSL_COMP_get_compression_methods());
    LOG_INFO("stop mc server...");
    evconnlistener_free(listener);

    event_base_free(base);

    objectID_table_destruct();
    db_destruct();
    curl_global_cleanup();
    zlog_fini();

    return 0;
}
