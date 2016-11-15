/*
 * msg_proc_http.h
 *
 *  Created on: 2016/10/13
 *      Author: lc
 */
#ifndef MSG_PROC_HISTORY_H_
#define MSG_PROC_HISTORY_H_

#include <http/server.hpp>
#include <http/request.hpp>
#include <http/reply.hpp>

#ifdef __cplusplus
    extern "C"{
#endif

http::server::reply history_getGPS(const char *imeiName, int starttime, int endtime);
http::server::reply history_getItinerary(const char *imeiName, int starttime, int endtime);
http::server::reply telephone_deleteTelNumber(const char *imeiName);
http::server::reply telephone_replaceTelNumber(const char *imeiName, const char *telNumber);
http::server::reply telephone_getTelNumber(const char *imeiName);
http::server::reply history_errorMsg(void);
http::server::reply history_okMsg(void);

#ifdef __cplusplus
    }
#endif

#endif /* MSG_PROC_HISTORY_H_ */

