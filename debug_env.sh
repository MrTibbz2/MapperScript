#!/bin/bash

echo "=== Environment Summary ==="
echo "Host: $(hostname) | Arch: $(uname -m) | Date: $(date '+%Y-%m-%d %H:%M')"
echo

echo "=== Key Differences ==="
echo "GCC: $(gcc --version | head -1 | cut -d')' -f2 | xargs)"
echo "Distro: $(grep PRETTY_NAME /etc/os-release 2>/dev/null | cut -d'=' -f2 | tr -d '"' || echo 'Unknown')"
echo "Lua System: $(lua -v 2>/dev/null || echo 'Not found')"
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH:-'(empty)'}"
echo

echo "=== Library Status ==="nARCH=$(uname -m)
VENDORED_LUA="./vendored/linux_lua/$ARCH/liblua54.so"
if [ -f "$VENDORED_LUA" ]; then
    echo "Vendored Lua ($ARCH): EXISTS ($(stat -c%s "$VENDORED_LUA" 2>/dev/null || echo '?') bytes)"
    echo "Vendored Lua deps: $(ldd "$VENDORED_LUA" 2>/dev/null | wc -l || echo '?') libraries"
else
    echo "Vendored Lua ($ARCH): NOT FOUND"
fi

if [ -f "./cmake-build-debug/MapperScript" ]; then
    echo "Executable: EXISTS"
    echo "Exe Lua deps: $(ldd ./cmake-build-debug/MapperScript 2>/dev/null | grep -i lua || echo 'None found')"
else
    echo "Executable: NOT FOUND"
fi
echo

echo "=== Build Config ==="
if [ -f "./cmake-build-debug/CMakeCache.txt" ]; then
    echo "CMake CXX Compiler: $(grep CMAKE_CXX_COMPILER:FILEPATH ./cmake-build-debug/CMakeCache.txt 2>/dev/null | cut -d'=' -f2 || echo 'Unknown')"
    echo "CMake Build Type: $(grep CMAKE_BUILD_TYPE ./cmake-build-debug/CMakeCache.txt 2>/dev/null | cut -d'=' -f2 || echo 'Unknown')"
    echo "Build System: $(grep CMAKE_GENERATOR: ./cmake-build-debug/CMakeCache.txt 2>/dev/null | cut -d'=' -f2 || echo 'Unknown')"
else
    echo "CMake cache: NOT FOUND"
fi

echo "Build files present:"
ls -1 ./cmake-build-debug/ 2>/dev/null | grep -E "Makefile|build.ninja|.*\.vcxproj" | head -3 || echo "None found"

if [ -d "./vendored/sol2/include" ]; then
    SOL_VER=$(grep -h "SOL_VERSION_MAJOR\|SOL_VERSION_MINOR\|SOL_VERSION_PATCH" ./vendored/sol2/include/sol/version.hpp 2>/dev/null | grep -o '[0-9]' | tr '\n' '.' | sed 's/..$//')
    echo "Sol2 Version: ${SOL_VER:-'Unknown'}"
else
    echo "Sol2: NOT FOUND"
fi
echo

echo "=== Critical Sol2 Test ==="
# Test the exact problematic case
cat > test_minimal.cpp << 'EOF'
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
EOF

echo "Compiling minimal test..."
ARCH=$(uname -m)
if [ -f "./vendored/linux_lua/$ARCH/liblua54.so" ]; then
    COMPILE_CMD="g++ -std=c++23 -I./vendored/sol2/include -I./vendored/linux_lua/$ARCH/include test_minimal.cpp -L./vendored/linux_lua/$ARCH -llua54 -ldl -o test_minimal 2>&1"
else
    COMPILE_CMD="g++ -std=c++23 -I./vendored/sol2/include test_minimal.cpp -llua5.4 -ldl -o test_minimal 2>&1"
fi

echo "Command: $COMPILE_CMD"
eval $COMPILE_CMD
if [ $? -eq 0 ]; then
    echo "Running test..."
    if [ -f "./vendored/linux_lua/$ARCH/liblua54.so" ]; then
        LD_LIBRARY_PATH="./vendored/linux_lua/$ARCH:$LD_LIBRARY_PATH" ./test_minimal
    else
        ./test_minimal
    fi
else
    echo "Compilation failed"
fi

echo
echo "=== Summary ==="
echo "Working Directory: $(pwd)"
echo "Project Files: $(ls -1 | wc -l) items"
echo "Plugins: $(ls -1 plugins/ 2>/dev/null | wc -l || echo 0) found"
echo "Scripts: $(ls -1 scripts/ 2>/dev/null | wc -l || echo 0) found"
echo "Build System Detected: $([ -f ./cmake-build-debug/Makefile ] && echo 'Make' || [ -f ./cmake-build-debug/build.ninja ] && echo 'Ninja' || echo 'Unknown')"
echo
echo "=== REPORT COMPLETE ==="