// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#include "webInterface/webinterface.h"

void WebManager::setupMethods()
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
        crow::response res(200, response);
        res.set_header("Content-Type", "application/json");
        return res;
    });
    
    // WebSocket logs endpoint
    CROW_ROUTE(app_, "/logs").websocket(&app_)
    .onopen([this](crow::websocket::connection& conn) {
        std::lock_guard<std::mutex> lock(wsMutex_);
        wsConnections_.insert(&conn);
    })
    .onclose([this](crow::websocket::connection& conn, const std::string&, uint16_t) {
        std::lock_guard<std::mutex> lock(wsMutex_);
        wsConnections_.erase(&conn);
    })
    .onmessage([](crow::websocket::connection&, const std::string&, bool) {
        // No handling needed for incoming messages
    });
    
    // Optional: Health check endpoint
    CROW_ROUTE(app_, "/health").methods("GET"_method)
    ([]() {
        return crow::response(200, R"({"status":"ok"})", "application/json");
    });
}

// Placeholder implementations - you'll implement these
json WebManager::handleUploadScript(const json& params) {
    // params[0] is a dict containing script data
    if (!params.is_array() || params.empty() || !params[0].is_object()) {
        return json{{"success", false}, {"message", "Invalid parameters"}};
    }
    
    const json& scriptData = params[0];
    std::string path = scriptData.value("path", "");
    std::string content = scriptData.value("content", "");
    
    if (path.empty() || content.empty()) {
        return json{{"success", false}, {"error", "Missing path or content"}};
    }
    
    // Use path as the script name and load content directly
    auto result = sm_.load_script(path, content);
    if (result == ScriptManager::SMLoadResult::FILE_LOAD_SUCCESS) {
        return json{{"success", true}, {"message", "Script loaded successfully"}};
    } else {
        return json{{"success", false}, {"message", "Failed to load script"}};
    }
}

json WebManager::handleGetScripts(const json& params) { // complete
    auto ScriptList = json::array();
    auto RawScripts = sm_.GetScripts();
    for (const auto& script : RawScripts) {
        ScriptList.push_back({
            {"name", script.name},
            {"path", script.path.string()},
            {"content", script.content}
        });
    }
    return ScriptList;
}

json WebManager::handleUpdateScript(const json& params) {
    // params[0] is a dict containing script path and content
    if (!params.is_array() || params.empty() || !params[0].is_object()) {
        return json{{"success", false}, {"error", "Invalid parameters"}};
    }
    
    const json& scriptData = params[0];
    std::string path = scriptData.value("path", "");
    std::string content = scriptData.value("content", "");
    
    if (path.empty() || content.empty()) {
        return json{{"success", false}, {"error", "Missing path or content"}};
    }
    
    auto result = sm_.update_script(std::filesystem::path(path), content);
    if (result == ScriptManager::SMLoadResult::FILE_LOAD_SUCCESS) {
        return json{{"success", true}, {"message", "Script updated successfully"}};
    } else {
        return json{{"success", false}, {"error", "Failed to update script"}};
    }
}

json WebManager::handleRunScript(const json& params) {
    // params[0] should contain script path
    if (!params.is_array() || params.empty()) {
        return json{{"success", false}, {"error", "Missing script path"}};
    }
    
    std::string path = params[0];
    auto future = sm_.run_script(std::filesystem::path(path));
    if (future.valid()) {
        return json{{"success", true}, {"message", "Script started successfully"}};
    } else {
        return json{{"success", false}, {"error", "Script already running or failed to start"}};
    }
}

json WebManager::handleGetPluginsList(const json& params) {
    auto pluginsList = json::array();
    const auto& plugins = pm_.GetAllPlugins();
    for (const auto& plugin : plugins) {
        pluginsList.push_back({
            {"name", plugin.name},
            {"loaded", plugin.loaded}
        });
    }
    return pluginsList;
}

json WebManager::handleGetPlugin(const json& params) {
    auto RequestedPluginName = params[0].get<std::string>();
    auto result = pm_.GetPluginByName(RequestedPluginName);
    
    if (result.has_value()) {
        return pluginToJson(result.value().get());
    }
    return json{{"success", false}, {"error", "Plugin not found"}};

}

json WebManager::handleGetStatus(const json& params) {
    return json{{"status", "ok"}}; // whatever, dont need this that much.
}



json WebManager::pluginToJson(const PluginManager::plugin& plugin) {
    return json{
        {"name", plugin.name},
        {"version", plugin.version},
        {"description", plugin.description},
        {"loaded", plugin.loaded},
        {"dependencies", plugin.dependencies},


    };
}