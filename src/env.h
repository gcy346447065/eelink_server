/*
 * env.h
 *
 *  Created on: May 27, 2015
 *      Author: jk
 */

#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include "curl.h"

typedef struct
{
  char *memory;
  size_t size;
}MemroyBuf;


typedef struct
{
    	struct mosquitto *mosq;

	CURL* curl_leancloud;
	CURL* curl_yeelink;
	MemroyBuf chunk;
}ENVIRONMENT;

void env_initial();
void env_cleanup();

ENVIRONMENT* env_get();

void env_resetChunk(MemroyBuf* chunk);

#endif /* SRC_ENV_H_ */
