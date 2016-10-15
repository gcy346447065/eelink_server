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
#include "msg_proc_http.h"
#include "protocol_history.h"

using namespace std;
http::server::reply history_reply(const http::server::request &req)
{
    string rsp;

    cout<<req.uri<<endl;//TODO: set a protocol and proc it

    char *gps = history_getGPS("865067021652600", 146000001, 146000006);
    history_freeMsg(gps);

    rsp += gps;
    http::server::reply rep(rsp);
    rep.headers["Content-Type"] = "text/plain";
    return rep;
}

int main(int argc, char *argv[])
{
  // Initialise the server.
  using namespace http::server;

  if(db_initial())
  {
      printf("connect to mysql failed\r\n");
      return -1;
  }

  server s("0.0.0.0", "8081", 5);

  s.add_handler("/history",history_reply);

  // Run the server until stopped.
  s.run();
  return 0;
}
