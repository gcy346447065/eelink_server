#include <event2/event.h>
#include <mosquitto.h>
#include <curl/curl.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <event2/listener.h>
#include <evhttp.h>
#include <timer.h>

#include "log.h"
#include "version.h"
#include "server_simcom.h"
#include "object.h"
#include "mqtt.h"
#include "db.h"
#include "msg_proc_app.h"
#include "setting.h"
#include "sync.h"
#include "session.h"
#include "http_simcom.h"
#include "redis.h"
#include "msg_proc_simcom.h"

#define isZeroTime_East8(time) (time % 86400 >= 16 * 3600 && time % 86400 < 16 * 3600 + 60) // 00:00 -> 16:00
#define isNoonTime_East8(time) (time % 86400 >= 4 * 3600 && time % 86400 < 4 * 3600 + 60) // 12:00 -> 4:00

static void signal_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    struct event_base *base = arg;

    LOG_WARN("server being stopped");

    event_base_loopbreak(base);
}

static void one_minute_loop_cb(evutil_socket_t fd __attribute__((unused)), short what __attribute__((unused)), void *arg)
{
    LOG_INFO("one minutes timer for ItieraryJudge_cb");

    obj_table_ItieraryJudge((void *)db_saveiItinerary); //simcom_server to judge if the itinerary reaches end


    time_t current = get_time();// everyday at 00:00, simcom_server to judge if devices need to be upgraded
    if(isZeroTime_East8(current) || isNoonTime_East8(current))
    {
        obj_table_FirmwareUpgrade((void *)simcom_startUpgradeRequest);
    }

    return;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int rc = log_init("../conf/simcom_log.conf");
    if (rc)
    {
        LOG_ERROR("log initial failed: rc=%d", rc);
    	return rc;
    }

    LOG_INFO("Electrombile Server %s, with event %s, mosquitto %d, curl %s\n",
    		VERSION_STR, LIBEVENT_VERSION, mosquitto_lib_version(NULL, NULL, NULL), curl_version());

    rc = setting_initail("../conf/eelink_server.ini");
    if (rc < 0)
    {
        LOG_ERROR("eelink_server.ini failed: rc=%d", rc);
    	return rc;
    }

    int simcom_port = setting.simcom_port;
    int http_port = setting.simhttp_port;


    struct event_base *base = event_base_new();
    if (!base)
    {
        return 1; /*XXXerr*/
    }

    struct event *evTerm = evsignal_new(base, SIGTERM, signal_cb, base);
    if (!evTerm || evsignal_add(evTerm, NULL) < 0)
    {
        LOG_ERROR("can't create SIGTERM event");
    }

    struct event *evInt = evsignal_new(base, SIGINT, signal_cb, base);
    if (!evInt || evsignal_add(evInt, NULL) < 0)
    {
        LOG_ERROR("can't create SIGINT event");
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
    LOG_INFO("HERE");
    rc = redis_initial();
    if (rc)
    {
    	LOG_ERROR("connect to redis failed");
    	return -1;
    }
    LOG_INFO("HERE");

    rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK)
    {
    	LOG_FATAL("curl lib initial failed:%d", rc);
    }
    LOG_INFO("HERE");

    rc = db_initial();
    if(rc)
    {
        LOG_FATAL("connect to mysql failed");
        return -1;
    }
    LOG_INFO("HERE");

    obj_table_initial(mqtt_subscribe, ObjectType_simcom);
    obj_table_GPSinitial();
    session_table_initial();
    LOG_INFO("HERE");

    struct evconnlistener *listener_simcom = server_simcom(base, simcom_port);
    if (listener_simcom)
    {
        LOG_INFO("start simcom server successfully at port:%d", simcom_port);
    }
    else
    {
        LOG_FATAL("start simcom server failed at port:%d", simcom_port);
        return 2;
    }

    //start a one minutes timer to resave multiple unsaved DIDs
    struct timeval one_min = { 60, 0 };
    (void)timer_newLoop(base, &one_min, one_minute_loop_cb, NULL);
    LOG_INFO("HERE");

    rc = sync_init(base);
    if (rc)
    {
        LOG_ERROR("connect to sync server failed, try later");
    }
    LOG_INFO("HERE");

	struct evhttp *httpd = evhttp_new(base);
	if (!httpd) {
		LOG_ERROR("couldn't create evhttp. Exiting.");
		return 1;
	}

    if (evhttp_bind_socket(httpd, "0.0.0.0", http_port) != 0)
    {
        LOG_ERROR("bind socket failed at port:%d", http_port);
        return 1;
    }

    evhttp_set_timeout(httpd, 5);
    evhttp_set_gencb(httpd, simcom_http_handler, NULL);

    //start the event loop
    LOG_INFO("start the event loop");
    event_base_dispatch(base);

    //sk_free(SSL_COMP_get_compression_methods());
    LOG_INFO("stop mc server...");

    sync_exit();

    evconnlistener_free(listener_simcom);

    evsignal_del(evTerm);
    evsignal_del(evInt);

    event_base_free(base);

    session_table_destruct();
    obj_table_destruct();

    db_destruct();
    curl_global_cleanup();

    mqtt_cleanup();

    mosquitto_lib_cleanup();

    zlog_fini();

    return 0;
}
