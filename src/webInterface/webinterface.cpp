// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#include "webInterface/webinterface.h"

void WebManager::setupMethods() const
{
    // Register JSON-RPC method handlers
    rpcServer_->Add("uploadScript", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleUploadScript(params);
    }));
    
    rpcServer_->Add("getScripts", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleGetScripts(params);
    }));

    rpcServer_->Add("updateScript", jsonrpccxx::MethodHandle([this](const json& params) -> json
    {
        return handleUpdateScript(params);
    }));

    
    rpcServer_->Add("runScript", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleRunScript(params);
    }));
    
    rpcServer_->Add("getPluginsList", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleGetPluginsList(params);
    }));
    
    rpcServer_->Add("getPlugin", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleGetPlugin(params);
    }));
    
    rpcServer_->Add("getStatus", jsonrpccxx::MethodHandle([this](const json& params) -> json {
        return handleGetStatus(params);
    }));
}

void WebManager::setupRoutes() {
    // Main JSON-RPC endpoint
    CROW_ROUTE(app_, "/rpc").methods("POST"_method)
    ([this](const crow::request& req) {
        const std::string response = rpcServer_->HandleRequest(req.body);
        return crow::response(200, response, "application/json");
    });
    
    // Optional: Health check endpoint
    CROW_ROUTE(app_, "/health").methods("GET"_method)
    ([]() {
        return crow::response(200, R"({"status":"ok"})", "application/json");
    });
}

// Placeholder implementations - you'll implement these
json WebManager::handleUploadScript(const json& params) {
    return json{{"success", false}, {"error", "Not implemented"}};
}

json WebManager::handleGetScripts(const json& params) {
    return json::array();
}

json WebManager::handleUpdateScript(const json& params) {
    return json{{"success", false}, {"error", "Not implemented"}};
}

json WebManager::handleRunScript(const json& params) {
    return json{{"success", false}, {"error", "Not implemented"}};
}

json WebManager::handleGetPluginsList(const json& params) {
    return json::array();
}

json WebManager::handleGetPlugin(const json& params) {
    return json{{"success", false}, {"error", "Not implemented"}};
}

json WebManager::handleGetStatus(const json& params) {
    return json{{"running", true}, {"message", "Not fully implemented"}};
}

json WebManager::scriptToJson(const std::filesystem::path& path) {
    return json{{"path", path.string()}, {"name", path.stem().string()}};
}

json WebManager::pluginToJson(const PluginManager::plugin& plugin) {
    return json{
        {"name", plugin.name},
        {"version", plugin.version},
        {"description", plugin.description},
        {"loaded", plugin.loaded}
    };
}