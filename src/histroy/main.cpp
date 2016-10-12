#include <iostream>
#include <string>

#include <http/server.hpp>
#include <http/request.hpp>
#include <http/reply.hpp>

#include "db.h"

using namespace std;
http::server::reply history_reply(const http::server::request &req)
{
    cout<<req.uri<<endl;
    db_getGPS("865067022405354", 1460520000, 1460529410);
    http::server::reply rep("hello world");
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
