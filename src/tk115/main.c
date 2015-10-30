#include <event2/event.h>
#include <mosquitto.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <event2/listener.h>

#include "log.h"
#include "version.h"
#include "object.h"
#include "server_tk115.h"
#include "yunba_push.h"
#include "db.h"
#include "session.h"
#include "mqtt.h"
#include "msg_proc_app.h"
#include "port.h"


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
    int port = PORT_TK115;

    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc >= 2)
    {
    	char* strPort = argv[1];
    	int num = atoi(strPort);
    	if (num)
    	{
    		port = num;
    	}
    }

    printf("Electrombile Server %s, with event %s, mosquitto %d\n",
    		VERSION_STR,
			LIBEVENT_VERSION,
			mosquitto_lib_version(NULL, NULL, NULL));

    base = event_base_new();
    if (!base)
        return 1; /*XXXerr*/

    int rc = log_init();
    if (rc)
    {
    	return rc;
    }

    if (signal(SIGINT, sig_usr) == SIG_ERR)
    {
        LOG_ERROR("Can't catch SIGINT");
    }
    if (signal(SIGTERM, sig_usr) == SIG_ERR)
    {
    	LOG_ERROR("Can't catch SIGTERM");
    }

    rc = mosquitto_lib_init();
    if (rc != MOSQ_ERR_SUCCESS)
    {
    	LOG_ERROR("mosquitto lib initial failed: rc=%d", rc);
    	return -1;
    }

    static MQTT_ARG mqtt_arg =
    {
            .app_msg_handler = app_handleApp2devMsg
    };
    mqtt_arg.base = base;

    mqtt_initial(&mqtt_arg);

    rc = yunba_connect();
    if (rc)
    {
    	LOG_FATAL("connect to yunba failed");
    	return -1;
    }

    rc = db_initial();
    if(rc)
    {
        LOG_FATAL("connect to mysql failed");
        return -1;
    }

    obj_table_initial(mqtt_subscribe);
    session_table_initial();

    struct evconnlistener* listener = server_start(base, port);
    if (listener)
    {
        LOG_INFO("start mc server successfully at port:%d", port);
    }
    else
    {
    	LOG_FATAL("start mc server failed at port:%d", port);
    	return 2;
    }


    //start the event loop
    LOG_INFO("start the event loop");
    event_base_dispatch(base);


    LOG_INFO("stop mc server...");
    evconnlistener_free(listener);
    event_base_free(base);

    session_table_destruct();
    obj_table_destruct();

    db_destruct();

    yunba_disconnect();

    mosquitto_lib_cleanup();


    zlog_fini();


    return 0;
}
