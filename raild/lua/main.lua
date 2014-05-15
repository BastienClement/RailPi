local s1 = Switch:Create(1, 1,  9, 17)
local s2 = Switch:Create(2, 2, 10, 18)
local s3 = Switch:Create(3, 3, 11, 19)
local s4 = Switch:Create(4, 4, 12, 20)
local s5 = Switch:Create(5, 5,  4, 21)

Switch:On("EnterC", function(switch)
    if switch.id == 3 then
        switch:Set(false)
    else
        switch:Toggle()
    end
end)

On("Ready", function()
    SetPower(true)
end)

local unconnected = { [8] = true, [16] = true, [24] = true, [13] = true, [23] = true }

On("SensorChange", function(sen, state)
    --if unconnected[sen] then return end
    print(sen, state)
end)
