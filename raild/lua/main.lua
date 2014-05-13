local s1 = Switch:Create(1, 1,  9, 17)
local s2 = Switch:Create(2, 2, 10, 18)
local s3 = Switch:Create(3, 3, 11, 19)
local s4 = Switch:Create(4, 4, 12, 20)
local s5 = Switch:Create(5, 5,  4, 21)

function toggleHandler(switch)
    return function()
        switch:Toggle()
    end
end

s1:On("EnterC", toggleHandler(s1))
s2:On("EnterC", toggleHandler(s2))
s4:On("EnterC", toggleHandler(s4))
s5:On("EnterC", toggleHandler(s5))

s3:On("EnterC", function()
    s3:Set(false)
end)

On("Ready", function()
    SetPower(true)
end)

local interest = { [6] = true, [7] = true, [8] = true, [14] = true, [15] = true, [16] = true, [22] = true, [23] = true, [23] = true,  }

On("SensorChange", function(sen, state)
    --if interest[sen] then return end
    --print(sen, state)
end)
