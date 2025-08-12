// /src/main.cpp

// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#include "MapperEngine.h"

[[noreturn]] int main() {


    const std::string testScriptPath = "/run/media/mrtibbs/OS/Users/lachl/CLionProjects/MapperScript/cmake-build-release/scripts/script.lua";
    ScriptManager ScriptMgr;
    ScriptMgr.init();

    ScriptMgr.start_watcher_thread();
    ScriptMgr.load_script(testScriptPath);

    const PluginManager PluginMgr;
    PluginMgr.loadPluginsFromDir("plugins", ScriptMgr);

    // Run script after plugins are loaded




    while (true)
    {
        std::string input;
        std::cout << "loaded. please press 1 to run script.\n";
        std::cin >> input;
        if (input == "1")
        {
            ScriptMgr.run_script(testScriptPath);
        }
    }

    ScriptMgr.stop_watcher_thread();
}
