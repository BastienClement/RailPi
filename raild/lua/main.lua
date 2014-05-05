
local lock_s1 = false
local lock_s2 = false

function do_lock_s2()
    lock_s2 = true
    CreateTimer(500, 0, function() lock_s2 = false end)
end

function watchdog()
    if IsPowered() and GetSensor(2) and GetSensor(10) then
        SetPower(false)
    elseif not IsPowered() and not (GetSensor(2) and GetSensor(10)) and GetSensor(1) then
        SetPower(true)
    end
end

On("SensorChanged", function(sid, state)
    print("Sensor", sid, state)
    watchdog()

    if not lock_s1 then
        if state and sid == 17 then
            SetSwitch(1, not GetSwitch(1))
            lock_s1 = true
            CreateTimer(500, 0, function() lock_s1 = false end)
        end
    end

    if not lock_s2 then
        if state and sid == 2 then
            SetSwitch(2, true)
            do_lock_s2()
        elseif state and sid == 10 then
            SetSwitch(2, false)
            do_lock_s2()
        end
    end
end)

CreateTimer(500, 500, function()
    watchdog()
end)

On("Ready", function()
    SetPower(true)
end)
