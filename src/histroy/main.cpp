/*
 * main.c
 *
 *  Created on: 2016/10/10
 *      Author: lc
 */

#include <iostream>
#include <string>

#include "msg_proc_http.hpp"
#include "db.h"
#include "log.h"

using namespace std;
http::server::reply history_reply(const http::server::request &req)
{
    int rc;
    int start = 0, end = 0;
    char imei[IMEI_LENGTH + 1] = {0};

    LOG_INFO("%s",req.uri.c_str());

    rc = sscanf(req.uri.c_str(), "/v1/history/%15s?start=%d&end=%d%*s", imei, &start, &end);
    if(rc == 3)
    {
        return history_getGPS(imei, start, end);
    }

    return history_errorMsg();
}

http::server::reply itinerary_reply(const http::server::request &req)
{
    int rc;
    int start = 0, end = 0;
    char imei[IMEI_LENGTH + 1] = {0};

    LOG_INFO("%s",req.uri.c_str());

    rc = sscanf(req.uri.c_str(), "/v1/itinerary/%15s?start=%d&end=%d%*s", imei, &start, &end);
    LOG_INFO("%s", req.method.c_str());
    if(rc == 3)
    {
        return history_getItinerary(imei, start, end);
    }

    return history_errorMsg();
}

http::server::reply telephone_reply(const http::server::request &req)
{
    int rc;
    char imei[IMEI_LENGTH + 1] = {0};
    char telNumber[TELNUMBER_LENGTH + 1] = {0};
    LOG_INFO("%s",req.uri.c_str());

    if(req.method == "POST" || req.method == "PUT")// set or update
    {
        rc = sscanf(req.uri.c_str(), "/v1/telephone/%15s?telephone=%11s%*s", imei, telNumber);
        if(rc == 2)
        {
            return telephone_replaceTelNumber(imei, telNumber);
        }
    }

    if(req.method == "DELETE")//delete
    {
        rc = sscanf(req.uri.c_str(), "/v1/telephone/%15s%*s", imei);
        if(rc == 1)
        {
            return telephone_deleteTelNumber(imei);
        }
    }

    if(req.method == "GET")//get
    {
        rc = sscanf(req.uri.c_str(), "/v1/telephone/%15s%*s", imei);

        if(rc == 1)
        {
            return telephone_getTelNumber(imei);
        }
    }

    return history_errorMsg();
}


int main(int argc, char *argv[])
{
  // Initialise the server.
  using namespace http::server;
  int rc;

  rc = db_initial();
  if(rc)
  {
      printf("connect to mysql failed\r\n");
      return -1;
  }

  rc = log_init("../conf/history_log.conf");
  if (rc)
  {
      LOG_ERROR("log initial failed: rc=%d", rc);
      return rc;
  }

  server s("0.0.0.0", "8081", 1);

  s.add_handler("/v1/history",history_reply);
  s.add_handler("/v1/itinerary",itinerary_reply);
  s.add_handler("/v1/telephone",telephone_reply);

  // Run the server until stopped.
  LOG_INFO("history server start.");
  s.run();
  return 0;
}
