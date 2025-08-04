local Module = {}

function Module.add_two_numbers(num1, num2)
    return num1 + num2
end

function Module.cpp_add_two_nums(a, b)
    return cpp_add_two_numbers(a, b)
end

return Module