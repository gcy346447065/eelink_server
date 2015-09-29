/*
 * yeelink_rsp.h
 *
 *  Created on: Apr 30, 2015
 *      Author: jk
 */

#ifndef SRC_YEELINK_RSP_H_
#define SRC_YEELINK_RSP_H_

#include <stdio.h>


size_t yeelink_onCreateDevice(void *contents, size_t size, size_t nmemb, void *userp);
size_t yeelink_onCreateSensor(void *contents, size_t size, size_t nmemb, void *userp);
size_t yeelink_onsaveGPS(void *contents, size_t size, size_t nmemb, void *userp);


#endif /* SRC_YEELINK_RSP_H_ */
