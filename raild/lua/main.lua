local s1 = CreateSmartSwitch(1, 1,  9, 17)
local s2 = CreateSmartSwitch(2, 2, 10, 18)
local s3 = CreateSmartSwitch(3, 3, 11, 19)
local s4 = CreateSmartSwitch(4, 4, 12, 20)
local s5 = CreateSmartSwitch(5, 5, 13, 21)
local s6 = CreateSmartSwitch(6, 6, 14, 22)
local s7 = CreateSmartSwitch(7, 7, 15, 23)

-- Switch 1 toggles
s1:setHandler(function()
    self:toggle()
end)

On("Ready", function()
    SetPower(true)
end)
