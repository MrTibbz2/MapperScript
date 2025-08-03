// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
#include <sol/sol.hpp>
#include <iostream>
#include "ScriptManager.h"
int main() {


    const std::string testScriptPath = "/run/media/mrtibbs/OS/Users/lachl/CLionProjects/MapperScript/cmake-build-release/scripts/script.lua";
    ScriptManager ScriptMgr;
    ScriptMgr.init();

    ScriptMgr.start_watcher_thread();
    ScriptMgr.load_script(testScriptPath);

    std::future Script_Exec = ScriptMgr.run_script(testScriptPath);
    while (true)
    {
        ScriptMgr.print_fileTimes();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    ScriptMgr.stop_watcher_thread();
}
