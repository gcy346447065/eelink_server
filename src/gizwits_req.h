#ifndef _SERVER_GIZWITS_
#define _SERVER_GIZWITS_

void mc_register(void* ctx);
void mc_provision(void* ctx);

void mc_login_mqtt(void* ctx);
int mqtt_dev2app(const char* topic, const char* data, const int len, void* ctx);


#endif //_SERVER_GIZWITS_
