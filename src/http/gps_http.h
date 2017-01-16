/*
 * gps_http.h
 *
 *  Created on: 2016/11/28
 *      Author: lc
 */
#ifndef SRC_GPS_HTTP_H_
#define SRC_GPS_HTTP_H_

#include <evhttp.h>

void http_replyGPS(struct evhttp_request *req, struct event_base *base __attribute__((unused)));



#endif/* SRC_GPS_HTTP_H_ */

