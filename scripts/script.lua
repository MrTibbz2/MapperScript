-- Copyright (c) 2025 Lachlan McKenna
-- All rights reserved. No part of this code may be used, copied, or distributed without permission.

-- Demonstrate the new Lua-based inter-plugin communication system
package.path = package.path .. ";../?.lua"

print("=== MapperScript Plugin Communication Demo ===")

-- Load plugin headers (these act as public APIs)
local test_plugin = require("plugins.test_plugin.plugin")
local math_consumer = require("plugins.math_consumer.plugin")

print("\n--- Testing test_plugin directly ---")
local sum = test_plugin.add(10, 15)
print("test_plugin.add(10, 15) = " .. sum)

local product = test_plugin.multiply(7, 8)
print("test_plugin.multiply(7, 8) = " .. product)

local complex = test_plugin.complex_math(3, 4, 5)  -- (3*4) + 5 = 17
print("test_plugin.complex_math(3, 4, 5) = " .. complex)

print("\n--- Testing math_consumer (depends on test_plugin) ---")

local mod = require("plugins.test_plugin.plugin")
print("Type of cpp_multiply:", type(mod.cpp_multiply))
local square = math_consumer.square(6)
print("math_consumer.square(6) = " .. square)

local sum_squares = math_consumer.sum_of_squares(3, 4)  -- 3^2 + 4^2 = 25
print("math_consumer.sum_of_squares(3, 4) = " .. sum_squares)

local complex_calc = math_consumer.complex_calculation(5)
print("math_consumer.complex_calculation(5) = " .. complex_calc)

print("\n--- Legacy compatibility test ---")
local legacy = test_plugin.cpp_add_two_nums(45, 45)
print("test_plugin.cpp_add_two_nums(45, 45) = " .. legacy)

print("\n=== Demo completed successfully ===")