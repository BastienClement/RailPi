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
--      (A) (B)
--      | |/ /
--      | / /     STATE      EXIT
--      |/ /      false      (A)
--      | /       true       (B)
--      | |
--      (C)
--
function CreateSwitch(sid, senA, senB, senC)
    -- Check if this switch is already registered
    if switches[sid] then
        error("called CreateSmartSwitch with an already registered switchID")
    end

    -- Bind event handler if not already done
    DoBind()

    -- Switch object
    local switch = EventEmitter({
        ["locked"] = false,
        ["exit_sensor"] = -1,
        ["state"] = false
    })

    -- Lock this switch in a given position
    function switch:Lock(state, exit)
        self:Set(state)
        self.locked = true
        self.exit_sensor = exit
        self:Emit("Lock", state, exit)
    end

    -- Free the previously set lock
    function switch:Unlock()
        self.locked = false
        self.exit_sensor = -1
        self:Emit("Unlock")
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
            self:Emit("Set", state)
        elseif not silent then
            error("attempted to set a locked switch")
        end
    end

    -- Toggle switch state
    function switch:Toggle(silent)
        self:Set(not self.state, silent)
    end

    -- Register this switch and its event handler
    local delayTimer
    switches[sid] = function(sen, state)
        -- Only react to related sensors
        if sen == senA
        or sen == senB
        or sen == senC then
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
                        if switch.exit_sensor == senC then
                            bad_sensor = switch.state and senA or senB
                        else
                            bad_sensor = switch.exit_sensor == senB and senA or senB
                        end
                        if sen == bad_sensor then
                            -- The current train has nothing to do with this
                            -- sensor, power-offing
                            SetPower(false)
                            print("detected activity on bad sensorID, power-offing")
                            switch:Emit("BadSensor", sen)
                        end
                    end
                end
            elseif state then
                -- Rising edge on any sensor
                if sen == senA then
                    -- (A) -> (C)
                    switch:Lock(false, 3)
                    switch:Emit("Enter1", state, exit)
                elseif sen == senB then
                    -- (B) -> (C)
                    switch:Lock(true, 3)
                    switch:Emit("Enter2", state, exit)
                else
                    -- (C) -> (A|B)
                    switch:Emit("Enter3", state, exit)

                    -- Then lock it to the current state
                    switch:Lock(switch.state, switch.state and senA or senB)
                end
            end
        end
    end

    -- Destroys a SmartSwitch
    -- The object should not be used afterward
    function switch:Disable()
        self:Emit("Disable", state, exit)
        switches[sid] = nil
    end

    -- Return the switch object
    return switch
end
