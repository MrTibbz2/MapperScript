#!/bin/bash

echo "=== COMPREHENSIVE Environment Comparison Script ==="
echo "Date: $(date)"
echo "Hostname: $(hostname)"
echo "User: $(whoami)"
echo

echo "=== System Info ==="
echo "Architecture: $(uname -m)"
echo "OS: $(uname -a)"
echo "Kernel: $(uname -r)"
echo "Distribution:"
cat /etc/os-release 2>/dev/null || echo "No os-release found"
echo "CPU Info:"
lscpu | grep -E "Model name|Architecture|CPU op-mode|Byte Order" 2>/dev/null || echo "lscpu failed"
echo "Memory:"
free -h 2>/dev/null || echo "free failed"
echo

echo "=== Compiler & Build Tools ==="
echo "GCC Full Version:"
gcc --version
echo "G++ Full Version:"
g++ --version
echo "CMake Full Version:"
cmake --version
echo "Make Version:"
make --version | head -1 2>/dev/null || echo "make not found"
echo "LD Version:"
ld --version | head -1 2>/dev/null || echo "ld not found"
echo "GCC Config:"
gcc -v 2>&1 | tail -10
echo "G++ Predefined Macros (C++23 related):"
echo | g++ -std=c++23 -dM -E - | grep -E "__cplusplus|__GNUC__|__VERSION__" 2>/dev/null || echo "Failed to get macros"
echo

echo "=== Environment Variables ==="
echo "PATH: $PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CMAKE_PREFIX_PATH: $CMAKE_PREFIX_PATH"
echo

echo "=== Library Info ==="
echo "Checking Lua library:"
file /home/ubuntu/CLionProjects/MapperScript/vendored/linux_lua/aarch64/liblua54.so
echo "Library size and permissions:"
ls -la /home/ubuntu/CLionProjects/MapperScript/vendored/linux_lua/aarch64/liblua54.so
echo "Library dependencies:"
ldd /home/ubuntu/CLionProjects/MapperScript/vendored/linux_lua/aarch64/liblua54.so 2>/dev/null || echo "ldd failed"
echo "Library symbols (first 20):"
nm -D /home/ubuntu/CLionProjects/MapperScript/vendored/linux_lua/aarch64/liblua54.so 2>/dev/null | head -20 || echo "nm failed"
echo "Checking for system Lua:"
which lua 2>/dev/null || echo "System lua not found"
lua -v 2>/dev/null || echo "System lua version check failed"
echo

echo "=== Build Info ==="
echo "Checking if executable exists:"
ls -la /home/ubuntu/CLionProjects/MapperScript/cmake-build-debug/MapperScript 2>/dev/null || echo "Executable not found"
echo "Executable dependencies:"
ldd /home/ubuntu/CLionProjects/MapperScript/cmake-build-debug/MapperScript 2>/dev/null || echo "ldd failed"
echo "Executable symbols (lua related):"
nm /home/ubuntu/CLionProjects/MapperScript/cmake-build-debug/MapperScript 2>/dev/null | grep -i lua | head -10 || echo "nm failed or no lua symbols"
echo

echo "=== CMake Configuration ==="
echo "CMake cache contents:"
cat /home/ubuntu/CLionProjects/MapperScript/cmake-build-debug/CMakeCache.txt 2>/dev/null | grep -E "CMAKE_CXX_COMPILER|CMAKE_C_COMPILER|CMAKE_BUILD_TYPE|CMAKE_CXX_FLAGS" || echo "CMakeCache.txt not found"
echo "CMake configure log (last 50 lines):"
tail -50 /home/ubuntu/CLionProjects/MapperScript/cmake-build-debug/CMakeFiles/CMakeConfigureLog.yaml 2>/dev/null || echo "CMake configure log not found"
echo

echo "=== Sol2 Headers Check ==="
echo "Sol2 version info:"
grep -r "SOL_VERSION" /home/ubuntu/CLionProjects/MapperScript/vendored/sol2/include/ 2>/dev/null | head -5 || echo "Sol2 version not found"
echo "Sol2 config check:"
ls -la /home/ubuntu/CLionProjects/MapperScript/vendored/sol2/include/sol/ | head -10
echo

echo "=== Detailed Sol2 Tests ==="
cd /home/ubuntu/CLionProjects/MapperScript

# Test 1: Basic Sol2
cat > test_sol2_basic.cpp << 'EOF'
#include <iostream>
#include "vendored/sol2/include/sol/sol.hpp"

int main() {
    std::cout << "Sol2 version: " << SOL_VERSION_MAJOR << "." << SOL_VERSION_MINOR << "." << SOL_VERSION_PATCH << std::endl;
    std::cout << "Lua version: " << LUA_VERSION_MAJOR << "." << LUA_VERSION_MINOR << std::endl;
    return 0;
}
EOF

# Test 2: The problematic case
cat > test_sol2_problem.cpp << 'EOF'
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
EOF

echo "Compiling Sol2 basic test..."
g++ -std=c++23 -I./vendored/sol2/include -I./vendored/linux_lua/aarch64/include test_sol2_basic.cpp -L./vendored/linux_lua/aarch64 -llua54 -ldl -o test_sol2_basic -v 2>&1
if [ $? -eq 0 ]; then
    echo "Running Sol2 basic test..."
    ./test_sol2_basic
else
    echo "Sol2 basic test compilation failed"
fi

echo
echo "Compiling Sol2 problem test..."
g++ -std=c++23 -I./vendored/sol2/include -I./vendored/linux_lua/aarch64/include test_sol2_problem.cpp -L./vendored/linux_lua/aarch64 -llua54 -ldl -o test_sol2_problem -v 2>&1
if [ $? -eq 0 ]; then
    echo "Running Sol2 problem test..."
    ./test_sol2_problem
else
    echo "Sol2 problem test compilation failed"
fi

echo
echo "=== Compilation Details ==="
echo "Preprocessor output (first 50 lines):"
g++ -std=c++23 -I./vendored/sol2/include -I./vendored/linux_lua/aarch64/include -E test_sol2_basic.cpp 2>/dev/null | head -50 || echo "Preprocessor failed"

echo
echo "=== Runtime Environment ==="
echo "Current working directory: $(pwd)"
echo "Directory contents:"
ls -la
echo "Plugins directory:"
ls -la plugins/ 2>/dev/null || echo "No plugins directory"
echo "Scripts directory:"
ls -la scripts/ 2>/dev/null || echo "No scripts directory"
echo

echo "=== Final System State ==="
echo "Process limits:"
ulimit -a 2>/dev/null || echo "ulimit failed"
echo "Disk space:"
df -h . 2>/dev/null || echo "df failed"
echo

echo "=== END COMPREHENSIVE REPORT ==="
echo "Report completed at: $(date)"