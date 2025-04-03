#pragma once

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Poco/JSON/Array.h>
#include "EbikeHandler.h"

class WebServer {
public:
    WebServer(Poco::JSON::Array::Ptr& ebikes);
    void start(int port);

private:
    Poco::JSON::Array::Ptr& _ebikes;
};

#endif // WEBSERVER_H