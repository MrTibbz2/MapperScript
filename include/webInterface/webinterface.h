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

using json = nlohmann::json;

class WebManager {
public:
    WebManager(ScriptManager& sm, const PluginManager& pm, const uint16_t port = 18080)
        : sm_(sm), pm_(pm), port_(port), running_(false), rpcServer_(std::make_unique<jsonrpccxx::JsonRpc2Server>())
    {
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

    void log(const std::string& message) { // adds timestamped log entry, maintains max 1000 entries
        std::lock_guard<std::mutex> lock(logMutex_);
        logs_.push_back({std::time(nullptr), message});
        if (logs_.size() > 1000) logs_.pop_front();
    }

private:
    struct LogEntry { // represents a single log entry with timestamp
        std::time_t timestamp; // when the log entry was created
        std::string message; // the actual log message content
    };

    crow::SimpleApp app_; // HTTP server for transport layer
    std::unique_ptr<jsonrpccxx::JsonRpc2Server> rpcServer_; // JSON-RPC protocol handler
    ScriptManager& sm_; // Reference to script manager for script operations
    const PluginManager& pm_; // Reference to plugin manager for plugin info
    uint16_t port_; // HTTP server port
    std::thread serverThread_; // Background thread for HTTP server
    std::atomic<bool> running_; // Server running state flag
    std::deque<LogEntry> logs_; // Circular buffer for application logs
    std::mutex logMutex_; // Thread safety for log operations

    void setupMethods() const; // registers all JSON-RPC method handlers with the RPC server
    void setupRoutes(); // configures HTTP routes, primarily the /rpc endpoint for JSON-RPC
    
    // RPC method handlers. if a method name is already used the call will be rejected.
    static json handleUploadScript(const json& params); // uploads a lua script over json-rpc. gets loaded into the scripts vector
    // will also put script content in a fresh vector because yay
    static json handleGetScripts(const json& params); // returns all the loaded scripts, paths and names (if name exists)
    static json handleUpdateScript(const json& params); // updates content by name.
    static json handleRunScript(const json& params); // runs a script by name.
    static json handleGetPluginsList(const json& params); // returns all loaded plugins.
    static json handleGetPlugin(const json& params); // returns data of a specific plugin by name.
    static json handleGetStatus(const json& params); // gets the status of the MapperScript App.
    // NOTE: scripts will be stored on frontend. all scripts will get temporarily sent over for running.
    // scripts can be updated either with hot reload on frontend or manual reload  button.
    // Utility functions
    json scriptToJson(const std::filesystem::path& path); // converts script file info to JSON format for API responses.
    json pluginToJson(const PluginManager::plugin& plugin); // takes all loaded metadata and outputs json of it.
};