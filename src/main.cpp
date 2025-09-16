// /src/main.cpp
// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#include "MapperEngine.h"
#include "webInterface/webinterface.h"
#include <cstdlib>

[[noreturn]] int main() {


    const std::string testScriptPath = "./scripts/script.lua";
    ScriptManager ScriptMgr;
    ScriptMgr.init();

    ScriptMgr.start_watcher_thread();
    ScriptMgr.load_script(testScriptPath);

    const PluginManager PluginMgr;
    PluginMgr.loadPluginsFromDir("plugins", ScriptMgr);


    WebManager WebMgr(ScriptMgr, PluginMgr);
    //
    //
    //
    // WebMgr.run_async();

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

    
    std::exit(0);  // Force immediate exit, skip destructors.
    // this is used because im lazy, but also because plugins shouldn't keep persistent data, and if they do they should sync it manually.
}
