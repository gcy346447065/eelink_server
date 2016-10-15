/*
 * main.c
 *
 *  Created on: 2016/10/10
 *      Author: lc
 */

#include <iostream>
#include <string>

#include <http/server.hpp>
#include <http/request.hpp>
#include <http/reply.hpp>

#include "db.h"
#include "log.h"
#include "msg_proc_http.h"
#include "protocol_history.h"

using namespace std;
http::server::reply history_reply(const http::server::request &req)
{
    int rc;
    string rsp;
    int start = 0;
    int end = 0;
    char imei[IMEI_LENGTH + 1] = {0};
    LOG_INFO("%s",req.uri.c_str());
    rc = sscanf(req.uri.c_str(), "/v1/history/%15s?start=%d&end=%d", imei, &start, &end);
    if(rc == 3)
    {
        char *gps = history_getGPS(imei, start, end);
        rsp += gps;
        history_freeMsg(gps);
    }
    else
    {
        rsp += "error:your uri is not matched";
    }

    LOG_INFO("%s\r\n",rsp.c_str());
    http::server::reply rep(rsp);
    rep.headers["Content-Type"] = "text/plain";
    return rep;
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

  server s("0.0.0.0", "8081", 5);

  s.add_handler("/v1/history",history_reply);

  // Run the server until stopped.
  LOG_INFO("history server start.");
  s.run();
  return 0;
}
