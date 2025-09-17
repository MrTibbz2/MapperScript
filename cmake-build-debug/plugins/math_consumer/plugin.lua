--- math_consumer.lua - Public API Header
--- This plugin provides advanced math operations built on test_plugin's basic functions

local Module = {}

-- Access to dependency
local test_plugin = require("plugins.test_plugin.plugin")

--- Calculate the square of a number (x^2)
function Module.square(x)
    return math_consumer.power_of_two(x)
end

--- Calculate sum of squares: a^2 + b^2
function Module.sum_of_squares_lua(a, b)
    return math_consumer.sum_of_squares(a, b)  -- Calls C++ function
end

--- Direct access to C++ functions
function Module.power_of_two(x)
    return math_consumer.power_of_two(x)
end

function Module.sum_of_squares(a, b)
    return math_consumer.sum_of_squares(a, b)
end

--- Calculate distance using Pythagorean theorem
function Module.distance(a, b)
    return Module.sum_of_squares(a, b)
end

--- Demonstrate chaining multiple dependency calls
function Module.complex_calculation(x)
    local doubled = test_plugin.add(x, x)
    local squared = Module.square(doubled)
    return squared
end

return Module