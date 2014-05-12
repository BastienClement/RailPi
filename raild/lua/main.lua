local s1 = CreateSwitch(1, 0, 0, 0)

On("SensorChange", function(sen, state)
    print("Sensor", sen, "to", state)
    print("Set switch to", not GetSwitch(5))
    SetSwitch(5, not GetSwitch(5))
end)

do return end

local s1 = CreateSwitch(1, 1,  9, 17)
local s2 = CreateSwitch(2, 2, 10, 18)
local s3 = CreateSwitch(3, 3, 11, 19)
local s4 = CreateSwitch(4, 4, 12, 20)
local s5 = CreateSwitch(5, 5, 13, 21)
local s6 = CreateSwitch(6, 6, 14, 22)
local s7 = CreateSwitch(7, 7, 15, 23)

function toggleHandler(self)
    return function()
        self:Toggle()
    end
end

s1:SetHandler(toggleHandler(s1))
s2:SetHandler(toggleHandler(s2))
s4:SetHandler(toggleHandler(s4))
s5:SetHandler(toggleHandler(s5))

On("SensorChange", function(sen, state)
    print(sen, state)
end)

On("Ready", function()
    SetPower(true)
end)
