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

    std::future Script_Exec = ScriptMgr.run_script(testScriptPath);

    const PluginManager PluginMgr;

    PluginMgr.loadPluginsFromDir("plugins", ScriptMgr);

    while (true)
    {
        //ScriptMgr.print_fileTimes();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    ScriptMgr.stop_watcher_thread();
}
