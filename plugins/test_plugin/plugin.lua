--- test_plugin.lua - Public API Header
--- This file serves as the public interface for test_plugin
--- Other plugins and scripts should use these functions to interact with test_plugin

local Module = {}

-- Wrapper functions that call C++ implementations
function Module.add(a, b)
    return test_plugin.cpp_add(a, b)
end

function Module.multiply(a, b)
    return test_plugin.cpp_multiply(a, b)
end

-- Direct access to C++ functions
function Module.cpp_add(a, b)
    return test_plugin.cpp_add(a, b)
end

function Module.cpp_multiply(a, b)
    return test_plugin.cpp_multiply(a, b)
end

-- Legacy compatibility
function Module.cpp_add_two_nums(a, b)
    return cpp_add_two_numbers(a, b)
end

-- Complex operations
function Module.complex_math(x, y, z)
    local product = Module.multiply(x, y)
    local result = Module.add(product, z)
    return result
end

return Module