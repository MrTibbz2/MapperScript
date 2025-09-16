#include <iostream>
#include "vendored/sol2/include/sol/sol.hpp"
int main() {
    try {
        sol::state lua;
        lua.open_libraries(sol::lib::base);
        sol::table t = lua["nonexistent"];  // The problematic line
        std::cout << "SUCCESS: Table access worked, valid=" << t.valid() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << std::endl;
        return 1;
    }
}
