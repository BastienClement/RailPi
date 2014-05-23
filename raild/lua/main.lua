Sensors.Enable(6, 7, 14, 15, 22)
Sensors.Disable(8, 16, 24, 13, 23)

Switches[1].Enable(1,  9, 17)
Switches[2].Enable(2, 10, 18)
Switches[3].Enable(3, 11, 19)
Switches[4].Enable(4, 12, 20)
Switches[5].Enable(5,  4, 21)

local segment = Chrono()
local diff = Chrono()
local mayday_diff = Chrono()
local ok = true
local mayday = false
local slow_loco = false
local round = 0
local counter = false

Sensors[22].On("Rising", function()
    if counter then
        round = round + 1
        counter = false
        print("Tour:", round)
        RailMon.Send("Round", { count = round })
    else
        counter = true
    end
end)

Sensors[17].On("Rising", function()
    segment.Push()
end)

Switches[2].On("EnterC", function()
    local fast
    local t = segment.Shift()
    if t < segment.Mean() then
        print("Fast", t, segment.Mean())
        if mayday then
            print("Mayday!")
        elseif diff.Time() < (segment.Mean() * 2) then
            print("Too close")
            ok = false
        else
            print("It's okay!")
            ok = true
        end
        Switches[2].SetState(ok or mayday)
    else
        if mayday then
            print("Mayday!")
        else
            print("Slow", t, segment.Mean())
        end
        diff.Reset()
        slow_loco = true
        Switches[2].SetState(true and not mayday)
    end
end)

Switches[4].On("EnterC", function()
    if mayday then
        Switches[4].SetState(true)
    elseif slow_loco then
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
