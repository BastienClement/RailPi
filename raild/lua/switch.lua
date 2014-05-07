-- List of switches defined
local switches = {}

-- Bind the event handler on first CreateAutoSwitch() call
local DoBind
do
    local bound = false
    function DoBind()
        if bound then return else bound = true end
        On("SensorChanged", function(sid, state)
            for _, switch in pairs(switches) do
                switch:handleEvent(sid, state)
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
function CreateAutoSwitch(sid, sen1, sen2, sen3)
    -- Check if this switch is already registered
    if switches[sid] then
        error("called CreateAutoSwitch with an already registered switchID")
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
    function switch:lock(state, exit)
        self:set(state)
        self.locked = true
        self.exit_sensor = exit
    end

    -- Free the previously set lock
    function switch:unlock()
        self.locked = false
        self.exit_sensor = -1
    end

    -- Set the switch in a given position
    -- If the switch is locked, does nothing
    function switch:set(state, silent)
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

    -- Define the callback handler when sensor3 is enabled
    function switch:setHandler(fn)
        self.handler = fn
    end

    -- Handle sensor state changes
    local delayTimer
    function switch:handleEvent(sen, state)
        -- Only react to related sensors
        if sen == sen1
        or sen == sen2
        or sen == sen3 then
            if self.locked then
                if sen == self.exit_sensor then
                    -- Falling or rising edge on exit sensor
                    if not state then
                        -- Falling edge, delay unlock by 500ms
                        delayTimer = CreateTimer(500, 0, function()
                            switch:unlock()
                        end)
                    else
                        -- Rising edge, cancel the unlock delay
                        CancelTimer(delayTimer)
                    end
                else
                    if state then
                        -- Rising edge on bad sensor
                        local bad_sensor
                        if self.exit_sensor == sen3 then
                            bad_sensor = self.state and sen1 or sen2
                        else
                            bad_sensor = self.exit_sensor == sen2 and sen1 or sen2
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
                    self:lock(false, 3)
                elseif sen == sen2 then
                    -- (2) -> (3)
                    self:lock(true, 3)
                else
                    -- (3) -> (1|2)
                    if self.handler then
                        -- Call handler to set the switch
                        self.handler()
                    end

                    -- Then lock it to the current state
                    self:lock(self.state, self.state and sen1 or sen2)
                end
            end
        end
    end

    -- Register this switch and return it
    switches[sid] = switch
    return switch
end
