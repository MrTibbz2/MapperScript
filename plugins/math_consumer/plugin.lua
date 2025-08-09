--- math_consumer.lua - Public API Header
--- This plugin provides advanced math operations built on test_plugin's basic functions

local math_consumer = {}

--- Calculate the square of a number (x^2)
--- @param x number Base number
--- @return number x squared
function math_consumer.square(x)
    return math_consumer.power_of_two(x)
end

--- Calculate sum of squares: a^2 + b^2
--- @param a number First number
--- @param b number Second number
--- @return number Sum of squares
function math_consumer.sum_of_squares(a, b)
    return math_consumer.sum_of_squares(a, b)  -- Calls C++ function
end

--- Calculate distance using Pythagorean theorem: sqrt(a^2 + b^2)
--- @param a number First side
--- @param b number Second side
--- @return number Hypotenuse length (approximated as sum of squares for demo)
function math_consumer.distance(a, b)
    -- For demo purposes, just return sum of squares
    -- In real implementation, you'd add a sqrt function
    return math_consumer.sum_of_squares(a, b)
end

--- Demonstrate chaining multiple dependency calls
--- @param x number Input value
--- @return number Complex calculation result
function math_consumer.complex_calculation(x)
    -- Use test_plugin functions directly through Lua
    local doubled = test_plugin.add(x, x)
    local squared = math_consumer.square(doubled)
    return squared
end

return math_consumer