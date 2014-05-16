Sensors.debounce = 5

Sensors.Enable(6, 7, 14, 15, 22)
Sensors.Disable(8, 16, 24, 13, 23)

Switches[1].Enable(1,  9, 17)
Switches[2].Enable(2, 10, 18)
Switches[3].Enable(3, 11, 19)
Switches[4].Enable(4, 12, 20)
Switches[5].Enable(5,  4, 21)

Switches.On("EnterC", function(switch)
    if switch.GetId() == 3 then
        switch.SetState(false)
    else
        --switch.SetState(not switch.GetState())
    end
end)

local loop = { 15, 7, 1, 17, 22, 18, 2, 5, 21 }
local i = 1
local ready = false
local counter = 0

Sensors.On("Change", function(sensor, state)
    local id = sensor.GetId()
    if state then
        if not ready then
            if id == loop[1] then
                i = 2
                ready = true
                print("start")
            end
        else
            if loop[i] ~= id then
                print("got:", id, "expected", loop[i])
                SetPower(false)
            else
                i = i + 1
                if i > #loop then
                    i = 1
                    counter = counter + 1
                    print("loop:", counter)
                end
            end
        end
    end
end)

On("Ready", function()
    SetPower(true)
end)
