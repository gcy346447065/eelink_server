#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>    //for struct evkeyvalq
#include <event.h>
#include <evhttp.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <event2/bufferevent_ssl.h>

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


/**
 * This callback is responsible for creating a new SSL connection
 * and wrapping it in an OpenSSL bufferevent.  This is the way
 * we implement an https server instead of a plain old http server.
 */
static struct bufferevent* bevcb (struct event_base *base, void *arg)
{
    SSL_CTX *ctx = (SSL_CTX *) arg;
    return bufferevent_openssl_socket_new (base, -1, SSL_new (ctx), BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
}

static void http_setup_certs (SSL_CTX *ctx, const char *certificate_chain, const char *private_key)
{
    LOG_INFO ("Loading certificate chain from '%s'\n" "and private key from '%s'\n", certificate_chain, private_key);

    if (1 != SSL_CTX_use_certificate_chain_file (ctx, certificate_chain))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_certificate_chain_file");

    if (1 != SSL_CTX_use_PrivateKey_file (ctx, private_key, SSL_FILETYPE_PEM))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_PrivateKey_file");

    if (1 != SSL_CTX_check_private_key (ctx))
        die_most_horribly_from_openssl_error ("SSL_CTX_check_private_key");
}

int main(int argc, char *argv[])
{
    int http_port = PORT_HTTP;
    char *httpd_listen = "0.0.0.0"; // local adress
    int httpd_timeout = 30;         // in seconds

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

    struct event *evTerm = evsignal_new(base, SIGTERM, signal_handler, base);
    if (!evTerm || evsignal_add(evTerm, NULL) < 0)
    {
        LOG_ERROR("can't create SIGTERM event");
    }

    struct event *evInt = evsignal_new(base, SIGINT, signal_handler, base);
    if (!evInt || evsignal_add(evInt, NULL) < 0)
    {
        LOG_ERROR("can't create SIGINT event");
    }

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
    evhttp_set_gencb(httpd, httpd_handler, NULL);


    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_SSLv2);
    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
     * We just hardcode a single curve which is reasonably decent.
     * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
    EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (! ecdh)
      die_most_horribly_from_openssl_error("EC_KEY_new_by_curve_name");
    if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
      die_most_horribly_from_openssl_error("SSL_CTX_set_tmp_ecdh");
    /* Find and set up our server certificate. */
    const char *certificate_chain = "server-certificate-chain.pem";
    const char *private_key = "server-private-key.pem";
    http_setup_certs(ctx, certificate_chain, private_key);
    evhttp_set_bevcb (httpd, bevcb, ctx);// This is the magic that lets evhttp use SSL.


    LOG_INFO("start http_server: %d", http_port);

    event_base_dispatch(base);

    LOG_INFO("stop http_server: %d", http_port);

    event_base_free(base);
    db_destruct();
    zlog_fini();
    evhttp_free(httpd);
    return 0;
}

