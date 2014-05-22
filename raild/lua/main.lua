Sensors.Enable(6, 7, 14, 15, 22)
Sensors.Disable(8, 16, 24, 13, 23)

Switches[1].Enable(1,  9, 17)
Switches[2].Enable(2, 10, 18)
Switches[3].Enable(3, 11, 19)
Switches[4].Enable(4, 12, 20)
Switches[5].Enable(5,  4, 21)

local segment = Chrono()
local diff = Chrono()
local ok = true
local slow_loco = false
local round = 0
local counter = false

Sensors[22].On("Edge", function(state)
    if state then
        if counter then
            round = round + 1
            counter = false
            print("Tour:", round)
            RailMon.Send("Round", { count = round })
        else
            counter = true
        end
    end
end)

Sensors[17].On("Edge", function(state)
    if state then
        segment.Push()
    end
end)

Switches[2].On("EnterC", function()
    local fast
    if segment.Shift() < segment.Mean() then
        print("Fast")
        if diff.Time() < (segment.Mean() * 1.5) then
            print("Too close")
            ok = false
        else
            print("It's okay!")
            ok = true
        end
        Switches[2].SetState(ok)
    else
        print("Slow")
        diff.Reset()
        slow_loco = true
        Switches[2].SetState(true)
    end
end)

Switches[4].On("EnterC", function()
    if slow_loco then
        Switches[4].SetState(not ok)
        slow_loco = false
    else
        Switches[4].SetState(false)
    end
end)

On("Ready", function()
    Switches[3].SetState(false)
    SetPower(true)
end)
