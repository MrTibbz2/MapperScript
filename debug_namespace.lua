-- Debug script to check namespace contents
print("=== Namespace Debug ===")

if test_plugin then
    print("test_plugin exists")
    if test_plugin.cpp_add then
        print("test_plugin.cpp_add exists")
        local result = test_plugin.cpp_add(1, 2)
        print("test_plugin.cpp_add(1, 2) = " .. result)
    else
        print("test_plugin.cpp_add is NIL")
    end
    
    if test_plugin.cpp_multiply then
        print("test_plugin.cpp_multiply exists")
    else
        print("test_plugin.cpp_multiply is NIL")
    end
else
    print("test_plugin namespace does NOT exist")
end

print("=== End Debug ===")