// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
// /src/webInterface/webinterface.h

#pragma once
#include "crow/crow_all.h"
class WebManager {
public:
    WebManager(const uint16_t port = 18080)
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



    // Start server in a separate thread
    void run_async()
    {
        if (running_) return;
        running_ = true;
        serverThread_ = std::thread([this](){
            app_.port(port_).multithreaded().run();
        });
    }

    // Stop server
    void stop()
    {
        if (!running_) return;
        app_.stop();           // requires Crow >=1.2
        if (serverThread_.joinable())
            serverThread_.join();
        running_ = false;
    }

private:
    crow::SimpleApp app_;
    uint16_t port_;
    std::thread serverThread_;
    std::atomic<bool> running_;

    static crow::response handleRoot(const crow::request& /*req*/) {
        return crow::response(200, "MapperScript Web Interface is up!");
    }
    static crow::response handleStatus(const crow::request& /*req8*/)
    {
        return crow::response(200, "MapperScript Web Interface is yeah!");
    }
};