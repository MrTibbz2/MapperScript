-- Test if C++ functions are accessible from script
print("Testing direct C++ function access:")

if test_plugin and test_plugin.cpp_add then
    local result = test_plugin.cpp_add(5, 3)
    print("SUCCESS: test_plugin.cpp_add(5, 3) = " .. result)
else
    print("FAILED: test_plugin.cpp_add is nil")
end