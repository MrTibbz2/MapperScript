// /src/main.cpp
// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#include "MapperEngine.h"
#include "webInterface/webinterface.h"
#include <cstdlib>
[[noreturn]] int main() {


    const std::string testScriptPath = "scripts/script.lua";
    ScriptManager ScriptMgr;
    ScriptMgr.init();

    ScriptMgr.start_watcher_thread();
    ScriptMgr.load_script(testScriptPath);

    const PluginManager PluginMgr;
    PluginMgr.loadPluginsFromDir("plugins", ScriptMgr);

    // Run script after plugins are loaded
    WebManager WebMgr;

    WebMgr.run_async();   // run blocking in a separate thread



    while (true)
    {

        std::string input;
        std::cout << "loaded. please press 1 to run script.\n";
        std::cin >> input;
        if (input == "1")
        {
            ScriptMgr.run_script(testScriptPath);
        }
        if (input == "2")
        {
            break;
        }

    }
    std::cout << "terminating watcher thread.\n";
    ScriptMgr.stop_watcher_thread();
    WebMgr.stop();
    
    std::exit(0);  // Force immediate exit, skip destructors
}
