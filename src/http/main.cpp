#include <httpserver.hpp>

#include "histroy_resource.h"

using namespace httpserver;

int main(int argc, char *argv[])
{
    webserver ws = create_webserver(8080).start_method(http::http_utils::INTERNAL_SELECT).max_threads(5);

    histroy_resource histroy;

    //TODO: the url must be change to regex
    ws.register_resource("/histroy", &histroy, true);

    ws.start(true);

    return 0;
}