// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
// /src/webInterface/webinterface.h

#pragma once
#include "crow/crow_all.h"
class WebManager {
public:
    WebInterface(const uint16_t port = 18080)
        : port_(port)
    {
        // Define your routes here
        app_.route_dynamic("/")( [this](const crow::request& req) {
            return handleRoot(req);
        });

        app_.route_dynamic("/status")( [this](const crow::request& req) {
            return handleStatus(req);
        });

        // Add more routes as member functions or lambdas
    }

    // Start the server, blocks until stopped
    void run() {
        app_.port(port_).multithreaded().run();
    }

    // You could add stop(), async run(), etc. as needed

private:
    crow::SimpleApp app_;
    uint16_t port_;

    static crow::response handleRoot(const crow::request& /*req*/) {
        return crow::response(200, "MapperScript Web Interface is up!");
    }
};