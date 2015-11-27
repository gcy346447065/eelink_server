#include <event2/event.h>
#include <mosquitto.h>
#include <curl/curl.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <event2/listener.h>

#include "log.h"
#include "version.h"
#include "server_simcom.h"
#include "yunba_push.h"
#include "object.h"
#include "mqtt.h"
#include "db.h"
#include "msg_proc_app.h"
#include "port.h"
#include "sync.h"


static void signal_cb(evutil_socket_t fd, short what, void *arg)
{
    struct event_base *base = arg;
    printf("oops! being stoped\n");

    event_base_loopbreak(base);
}

int main(int argc, char **argv)
{
    int simcom_port= PORT_SIMCOM;

    setvbuf(stdout, NULL, _IONBF, 0);


    if (argc >= 2)
    {
    	char* strPort = argv[1];
    	int num = atoi(strPort);
    	if (num)
    	{
    		simcom_port = num;
    	}
    }

    printf("Electrombile Server %s, with event %s, mosquitto %d, curl %s\n",
    		VERSION_STR,
			LIBEVENT_VERSION,
			mosquitto_lib_version(NULL, NULL, NULL),
			curl_version());

    struct event_base *base = event_base_new();
    if (!base)
        return 1; /*XXXerr*/

    int rc = log_init("../conf/simcom_log.conf");

    if (rc)
    {
        LOG_ERROR("log initial failed: rc=%d", rc);
    	return rc;
    }

    struct event *evTerm = evsignal_new(base, SIGTERM, signal_cb, base);
    if (!evTerm || evsignal_add(evTerm, NULL) < 0)
    {
        LOG_ERROR("can't create SIGTERM event");
    }

    struct event *evInt = evsignal_new(base, SIGINT, signal_cb, base);
    if (!evInt || evsignal_add(evInt, NULL) < 0)
    {
        LOG_ERROR("can't create SIGTERM event");
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

    obj_table_initial(mqtt_subscribe);
    session_table_initial();

    struct evconnlistener* listener_simcom = server_simcom(base, simcom_port);
    if (listener_simcom)
    {
        LOG_INFO("start simcom server successfully at port:%d", simcom_port);
    }
    else
    {
        LOG_FATAL("start simcom server failed at port:%d", simcom_port);
        return 2;
    }

    rc = sync_init(base);
    if (rc)
    {
        LOG_ERROR("connect to sync server failed, try later");
        //TODO: start a timer to re-connect to the sync server
    }

    //start the event loop
    LOG_INFO("start the event loop");
    event_base_dispatch(base);


//    sk_free(SSL_COMP_get_compression_methods());
    LOG_INFO("stop mc server...");
    evconnlistener_free(listener_simcom);

    evsignal_del(evTerm);
    evsignal_del(evInt);

    event_base_free(base);

    session_table_destruct();
    obj_table_destruct();

    db_destruct();
    curl_global_cleanup();

    mqtt_cleanup();
    yunba_disconnect();

    mosquitto_lib_cleanup();


    zlog_fini();


    return 0;
}
