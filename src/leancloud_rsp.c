/*
 * leancloud_rsp.c
 *
 *  Created on: May 1, 2015
 *      Author: jk
 */
#include <stdlib.h>

#include "leancloud_rsp.h"
#include "cb_ctx_mc.h"
#include "object_mc.h"
#include "cJSON.h"
#include "log.h"

size_t leancloud_onSaveGPS(void *contents, size_t size, size_t nmemb, void *userp)
{
	char* rsp = malloc(size * nmemb + 1);
	memcpy(rsp, contents, size * nmemb);
	rsp[size * nmemb] = 0;

	cJSON* json = cJSON_Parse(rsp);

	if (!json)
	{
		LOG_ERROR("error parse respone:%s", rsp);
	}
	else
	{
		LOG_INFO("response: %s", rsp);
	}

	cJSON_Delete(json);
	free(rsp);

	return size * nmemb;
}

size_t leancloud_onRev(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	MemroyBuf *mem = (MemroyBuf *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL)
	{
		/* out of memory! */
		LOG_ERROR("not enough memory (realloc returned NULL)");
		return 0;
	}

	LOG_DEBUG("receive %d bytes from leancloud", realsize);

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}
