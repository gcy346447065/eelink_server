//
// Created by jk on 16-9-12.
//

#ifndef ELECTROMBILE_HISTROY_RESOURCE_H
#define ELECTROMBILE_HISTROY_RESOURCE_H

#include <httpserver.hpp>

using namespace httpserver;

class histroy_resource : public http_resource{
    public:
    histroy_resource() {}

    ~histroy_resource() {}

    void render_GET(const http_request &req, http_response** res);
};


#endif //ELECTROMBILE_HISTROY_RESOURCE_H
