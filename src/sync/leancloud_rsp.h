/*
 * leancloud_rsp.h
 *
 *  Created on: May 1, 2015
 *      Author: jk
 */

#ifndef SRC_LEANCLOUD_RSP_H_
#define SRC_LEANCLOUD_RSP_H_

#include <stdio.h>

size_t leancloud_onSaveGPS(void *contents, size_t size, size_t nmemb, void *userp);

size_t leancloud_onRev(void *contents, size_t size, size_t nmemb, void *userp);

#endif /* SRC_LEANCLOUD_RSP_H_ */
