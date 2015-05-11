
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "gizwits_req.h"
#include "gizwits_rsp.h"
#include "http.h"
#include "mqtt.h"
#include "object_mc.h"
#include "log.h"

#ifdef WITH_CATEGORY

#define LOG_DEBUG(...) \
	zlog(cat[MOD_GIZWITS_REQ], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(...) \
	zlog(cat[MOD_GIZWITS_REQ], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARNNING(...) \
	zlog(cat[MOD_GIZWITS_REQ], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_WARNNING, __VA_ARGS__)

#define LOG_ERROR(...) \
	zlog(cat[MOD_GIZWITS_REQ], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(...) \
	zlog(cat[MOD_GIZWITS_REQ], __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
#endif

void mc_register(void* ctx)
{
	OBJ_MC* obj = ((CB_CTX*)ctx)->obj;

	HTTP_SESSION *session = malloc(sizeof(HTTP_SESSION));
	init_session(session, ctx, mc_register_rsp);

	const char* url = "http://api.gizwits.com/dev/devices";
	char data[200] = {0}; //TODO: fix magic number

	snprintf(data, 200, "product_key=%s&passcode=%s&mac=%s&type=normal", PRODUCT_KEY, obj->pwd, getMacFromIMEI(obj->IMEI));

	LOG_INFO("Register: data = %s", data);

	http_post(session, url, data);

}

void mc_provision(void* ctx)
{
	OBJ_MC* obj = ((CB_CTX*)ctx)->obj;

	HTTP_SESSION *session = malloc(sizeof(HTTP_SESSION));
	init_session(session, ctx, mc_provision_rsp);

	//TODO: change the magic number
	char url[100];
	snprintf(url, 100, "http://api.gizwits.com/dev/devices/%s", obj->DID);
	http_get(session, url);
}

void mc_login_mqtt(void* ctx)
{
	OBJ_MC* obj = ((CB_CTX*)ctx)->obj;
	struct mosquitto* mosq = mqtt_login(obj->DID, obj->m2m_host, obj->m2m_Port, obj);
	obj->mosq = mosq;
}


//TODO: 2 bytes cannot represent 65535
typedef struct _varc
{
	char var[2]; //参数的值
	char varcbty;//1by-4by
} varc;

static varc Tran2varc(short remainLen)
{
	varc Tmp;

    if (remainLen <= 127)
    {
        //fixed_header[0] = remainLen;
        Tmp.var[0] = remainLen;
        Tmp.varcbty = 1;
    }
    else
    {
        // first byte is remainder (mod) of 128, then set the MSB to indicate more bytes
        Tmp.var[0] = remainLen % 128;
        Tmp.var[0]=Tmp.var[0] | 0x80;
        // second byte is number of 128s
        Tmp.var[1] = remainLen / 128;
        Tmp.varcbty=2;
    }
    return Tmp;
}


int mqtt_dev2app(const char* topic, const char* data, const int len, void* userdata)
{
	OBJ_MC* obj = userdata;

	char header[100];
	int headerlen = 0;

    /*
     * data format:
     * 		header: 4b  0x00000003
     * 		varlen: 1-4b (the following data len)
     * 		flag:	1b
     * 		cmd:	2b
     * 		data:	max 65536
     */
    typedef struct
    {
    	char flag;
    	short cmd;
    }__attribute__((__packed__)) CMD;

	varc Uart_varatt = Tran2varc(len + sizeof(CMD));
	header[0] = 0x00;
	header[1] = 0x00;
	header[2] = 0x00;
	header[3] = 0x03;

	for(int i = 0;i < Uart_varatt.varcbty; i++)
	{
		header[4 + i] = Uart_varatt.var[i];
	}
	CMD* cmd = header + 4 + Uart_varatt.varcbty; 	// 4 is for the header signature 0x00000003
	cmd->flag = 0x00;
	cmd->cmd = htons(0x0091);
//	header[4 + Uart_varatt.varcbty] = 0x00;	//flag
//	header[4 + Uart_varatt.varcbty + 1] = 0x00;	//cmd
//	header[4 + Uart_varatt.varcbty + 2] = 0x91;

	headerlen = 4 + Uart_varatt.varcbty + sizeof(CMD);	//header(4) + varLen(x) + flag(1) + cmd(2);

	int payloadlen = headerlen + len;
	char* payload = malloc(payloadlen);
	if(payload)
	{
		memcpy(payload, header, headerlen);
		memcpy(payload + headerlen, data, len);
	}

    LOG_HEX(payload, payloadlen);

	//topic: "dev2app/<DID>"
	mosquitto_publish(obj->mosq, NULL, topic, payloadlen, payload, 0, false);	//TODO: determine the parameter

	//TODO: free the payload here may make problem on multithread enviroment
	free(payload);

	return 0;
}
