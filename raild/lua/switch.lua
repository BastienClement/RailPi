-- List of switches defined and their event handler
local switches = {}

-- Bind the event handler on first CreateSmartSwitch() call
local DoBind
do
    local bound = false
    function DoBind()
        if bound then return else bound = true end
        On("SensorChange", function(sid, state)
            for _, handler in pairs(switches) do
                handler(sid, state)
            end
        end)
    end
end

-- Creates a new auto-managed switch object
--
--      (1) (2)
--      | |/ /
--      | / /     STATE      EXIT
--      |/ /      false      (1)
--      | /       true       (2)
--      | |
--      (3)
--
function CreateSwitch(sid, sen1, sen2, sen3)
    -- Check if this switch is already registered
    if switches[sid] then
        error("called CreateSmartSwitch with an already registered switchID")
    end

    -- Bind event handler if not already done
    DoBind()

    -- Switch object
    local switch = {
        ["locked"] = false,
        ["exit_sensor"] = -1,
        ["state"] = false,
        ["handler"] = nil
    }

    -- Lock this switch in a given position
    function switch:Lock(state, exit)
        self:Set(state)
        self.locked = true
        self.exit_sensor = exit
    end

    -- Free the previously set lock
    function switch:Unlock()
        self.locked = false
        self.exit_sensor = -1
    end

    -- Set the switch in a given position
    -- If the switch is locked, does nothing
    function switch:Set(state, silent)
        -- Ignore call if switch is already is this position
        if self.state == state then return end

        -- Update internal state
        self.state = state

        -- If not locked, then send to RailHub
        if not self.locked then
            SetSwitch(sid, self.state)
        elseif not silent then
            error("attempted to set a locked switch")
        end
    end

    -- Toggle switch state
    function switch:Toggle(silent)
        self:Set(not self.state, silent)
    end

    -- Define the callback handler when sensor3 is enabled
    function switch:SetHandler(fn)
        self.handler = fn
    end

    -- Register this switch and its event handler
    local delayTimer
    switches[sid] = function(sen, state)
        -- Only react to related sensors
        if sen == sen1
        or sen == sen2
        or sen == sen3 then
            if switch.locked then
                if sen == switch.exit_sensor then
                    -- Falling or rising edge on exit sensor
                    if not state then
                        -- Falling edge, delay unlock by 500ms
                        delayTimer = CreateTimer(500, 0, function()
                            switch:Unlock()
                        end)
                    else
                        -- Rising edge, cancel the unlock delay
                        CancelTimer(delayTimer)
                    end
                else
                    if state then
                        -- Rising edge on bad sensor
                        local bad_sensor
                        if switch.exit_sensor == sen3 then
                            bad_sensor = switch.state and sen1 or sen2
                        else
                            bad_sensor = switch.exit_sensor == sen2 and sen1 or sen2
                        end
                        if sen == bad_sensor then
                            -- The current train has nothing to do with this
                            -- sensor, power-offing
                            SetPower(false)
                            print("detected activity on bad sensorID, power-offing")
                        end
                    end
                end
            elseif state then
                -- Rising edge on any sensor
                if sen == sen1 then
                    -- (1) -> (3)
                    switch:Lock(false, 3)
                elseif sen == sen2 then
                    -- (2) -> (3)
                    switch:Lock(true, 3)
                else
                    -- (3) -> (1|2)
                    if switch.handler then
                        -- Call handler to set the switch
                        switch.handler()
                    end

                    -- Then lock it to the current state
                    switch:Lock(switch.state, switch.state and sen1 or sen2)
                end
            end
        end
    end

    -- Destroys a SmartSwitch
    -- The object should not be used afterward
    function switch:Disable()
        switches[sid] = nil
    end

    -- Return the switch object
    return switch
end
