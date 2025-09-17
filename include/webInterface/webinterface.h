// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#pragma once
#include "crow/crow_all.h"
#include "server.hpp"
#include "dispatcher.hpp"
#include "Scripting/ScriptManager.h"
#include "plugins/PluginManager.h"
#include <nlohmann/json.hpp>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <ctime>
#include <set>

using json = nlohmann::json;

class WebManager {
public:
    WebManager(ScriptManager& sm, const PluginManager& pm, const uint16_t port = 18080)
        : sm_(sm), pm_(pm), port_(port), running_(false), rpcServer_(std::make_unique<jsonrpccxx::JsonRpc2Server>())
    {
        sm_.bind_function("log", [this](const std::string& msg) { wsLog(msg); });
        setupMethods();
        setupRoutes();
    }

    void run_async() {
        if (running_) return;
        running_ = true;
        serverThread_ = std::thread([this](){
            app_.port(port_).multithreaded().run();
        });
    }

    void stop() {
        if (!running_) return;
        app_.stop();
        if (serverThread_.joinable())
            serverThread_.join();
        running_ = false;
    }



    // WebSocket log helper function
    void wsLog(const std::string& message) {
        std::lock_guard<std::mutex> lock(wsMutex_);
        json logMsg = {{"timestamp", std::time(nullptr)}, {"message", message}};
        std::string msgStr = logMsg.dump();
        for (auto& conn : wsConnections_) {
            conn->send_text(msgStr);
        }
    }

private:


    crow::SimpleApp app_; // HTTP server for transport layer
    std::unique_ptr<jsonrpccxx::JsonRpc2Server> rpcServer_; // JSON-RPC protocol handler
    ScriptManager& sm_; // Reference to script manager for script operations
    const PluginManager& pm_; // Reference to plugin manager for plugin info
    uint16_t port_; // HTTP server port
    std::thread serverThread_; // Background thread for HTTP server
    std::atomic<bool> running_; // Server running state flag

    std::mutex logMutex_; // Thread safety for log operations
    
    // WebSocket members
    std::set<crow::websocket::connection*> wsConnections_;
    std::mutex wsMutex_;

    void setupMethods(); // registers all JSON-RPC method handlers with the RPC server
    void setupRoutes(); // configures HTTP routes, primarily the /rpc endpoint for JSON-RPC
    
    // RPC method handlers. if a method name is already used the call will be rejected.
    json handleUploadScript(const json& params); // uploads a lua script over json-rpc. gets loaded into the scripts vector
    // will also put script content in a fresh vector because yay
    json handleGetScripts(const json& params); // returns all the loaded scripts, paths and names (if name exists)
    json handleUpdateScript(const json& params); // updates content by name.
    json handleRunScript(const json& params); // runs a script by name.
    json handleGetPluginsList(const json& params); // returns all loaded plugins.
    json handleGetPlugin(const json& params); // returns data of a specific plugin by name.
    json handleGetStatus(const json& params); // gets the status of the MapperScript App.
    // NOTE: scripts will be stored on frontend. all scripts will get temporarily sent over for running.
    // scripts can be updated either with hot reload on frontend or manual reload  button.

    // Utility functions
    json pluginToJson(const PluginManager::plugin& plugin); // takes all loaded metadata and outputs json of it.
};