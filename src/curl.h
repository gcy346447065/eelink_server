/*
 * curl.h
 *
 *  Created on: Apr 30, 2015
 *      Author: jk
 */

#ifndef SRC_CURL_H_
#define SRC_CURL_H_

#include <curl/curl.h>

CURL* initCurlHandleOfLeancloud();
CURL* initCurlHandleOfYeelink();

#define cleanupLeancloudCurlHandle(curl) curl_easy_cleanup(curl)
#define cleanupYeelinkCurlHandle(curl) curl_easy_cleanup(curl)

void clcanupLeancloudHeader();
void cleanupYeelinkHeader();

#endif /* SRC_CURL_H_ */
