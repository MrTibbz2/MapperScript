#include <iostream>
#include "vendored/sol2/include/sol/sol.hpp"

int main() {
    try {
        std::cout << "Creating Sol2 state..." << std::endl;
        sol::state lua;
        std::cout << "Opening libraries..." << std::endl;
        lua.open_libraries(sol::lib::base);
        std::cout << "Testing problematic table access..." << std::endl;
        sol::table t = lua["nonexistent"];  // This should cause the error
        std::cout << "Table valid: " << t.valid() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Sol2 test failed: " << e.what() << std::endl;
        return 1;
    }
}
