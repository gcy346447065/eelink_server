#include <event2/event.h>
#include <mosquitto.h>
#include <curl/curl.h>
 #include <signal.h>
 #include <openssl/ssl.h>

#include "log.h"
#include "version.h"
#include "server_mc.h"

struct event_base *base = NULL;

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
}

int main(int argc, char **argv)
{

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Electrombile Server %s, with event %s, mosquitto %d, curl %s\n",
    		VERSION_STR,
			LIBEVENT_VERSION,
			mosquitto_lib_version(NULL, NULL, NULL),
			curl_version());

    base = event_base_new();
    if (!base)
        return 1; /*XXXerr*/

    int ret = log_init();
    if (ret)
    {
    	return ret;
    }

    mc_obj_initial();

    struct evconnlistener* listener = server_mc_start(base);
    if (listener)
    {
    	LOG_INFO("start mc server sucessfully");
    }
    else
    {
    	LOG_FATAL("start mc server failed");
    	return 2;
    }
    

    if (signal(SIGINT, sig_usr) == SIG_ERR)
    {
        LOG_ERROR("Can't catch SIGINT");
    }
    if (signal(SIGTERM, sig_usr) == SIG_ERR)
    {
    	LOG_ERROR("Can't catch SIGTERM");
    }
    mosquitto_lib_init();

    curl_global_init(CURL_GLOBAL_DEFAULT);


    //start the event loop
    event_base_dispatch(base);

    //cleanup all resouce
    mc_obj_destruct();
    evconnlistener_free(listener);
    event_base_free(base);
	mosquitto_lib_cleanup();
    curl_global_cleanup();
    clcanupLeancloudHeader();
    cleanupYeelinkHeader();

//    sk_free(SSL_COMP_get_compression_methods());
    LOG_INFO("stop mc server sucessfully");

    zlog_fini();

    return 0;
}
