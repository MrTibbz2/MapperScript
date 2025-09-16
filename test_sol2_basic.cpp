#include <iostream>
#include "vendored/sol2/include/sol/sol.hpp"

int main() {
    std::cout << "Sol2 version: " << SOL_VERSION_MAJOR << "." << SOL_VERSION_MINOR << "." << SOL_VERSION_PATCH << std::endl;
    std::cout << "Lua version: " << LUA_VERSION_MAJOR << "." << LUA_VERSION_MINOR << std::endl;
    return 0;
}
