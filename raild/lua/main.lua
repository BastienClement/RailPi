Sensors.Disable({ 8, 16, 24, 13, 23 })

Switches[1].Enable(1,  9, 17)
Switches[2].Enable(2, 10, 18)
Switches[3].Enable(3, 11, 19)
Switches[4].Enable(4, 12, 20)
Switches[5].Enable(5,  4, 21)

Switches.On("EnterC", function(switch)
    if switch.GetId() == 3 then
        switch.SetState(false)
    else
        switch.SetState(not switch.GetState())
    end
end)

Sensors.On("Change", function(sensor, state)
    print(sensor.GetId(), state)
end)

On("Ready", function()
    SetPower(true)
end)
