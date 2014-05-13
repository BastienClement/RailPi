Switch = EventEmitter({
    handlers = {},
    objects = {}
})

-- Bind the event handler on first CreateSmartSwitch() call
local DoBind
do
    local bound = false
    function DoBind()
        if bound then return else bound = true end
        On("SensorChange", function(sid, state)
            for _, handler in pairs(Switch.handlers) do
                handler(sid, state)
            end
        end)
    end
end

-- Return the switch object for a given SwitchID
function Switch:Get(sid)
    return self.objects[sid]
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
function Switch:Create(sid, senA, senB, senC)
    local handlers = self.handlers

    -- Check if this switch is already registered
    if handlers[sid] then
        error("called Switch:Create with an already registered switchID")
    end

    -- Bind event handler if not already done
    DoBind()

    -- Switch object
    local switch_obj = EventEmitter({
        ["locked"] = false,
        ["exit_sensor"] = -1,
        ["state"] = false
    })

    -- Emit events on both the switch object and the global
    local function emit(...)
        print("emit", ...)
        switch_obj:Emit(...)
        Switch:Emit(...)
    end

    -- Lock this switch in a given position
    function switch_obj:Lock(state, exit)
        self:Set(state)
        self.locked = true
        self.exit_sensor = exit
        emit("Lock", sid, state, exit)
    end

    -- Free the previously set lock
    function switch_obj:Unlock()
        self.locked = false
        self.exit_sensor = -1
        emit("Unlock", sid)
    end

    -- Set the switch in a given position
    -- If the switch is locked, does nothing
    function switch_obj:Set(state, silent)
        -- Ignore call if switch is already is this position
        if self.state == state then return end

        -- Update internal state
        self.state = state

        -- If not locked, then send to RailHub
        if not self.locked then
            SetSwitch(sid, self.state)
            emit("Set", sid, state)
        elseif not silent then
            error("attempted to set a locked switch")
        end
    end

    -- Toggle switch state
    function switch_obj:Toggle(silent)
        self:Set(not self.state, silent)
    end

    -- Register this switch and its event handler
    local delayTimer
    handlers[sid] = function(sen, state)
        -- Only react to related sensors
        if sen == senA
        or sen == senB
        or sen == senC then
            if switch_obj.locked then
                if sen == switch_obj.exit_sensor then
                    -- Falling or rising edge on exit sensor
                    if not state then
                        -- Falling edge, delay unlock by 100ms
                        delayTimer = CreateTimer(100, 0, function()
                            switch_obj:Unlock()
                        end)
                    else
                        -- Rising edge, cancel the unlock delay
                        CancelTimer(delayTimer)
                    end
                else
                    if state then
                        -- Rising edge on bad sensor
                        local bad_sensor
                        if switch_obj.exit_sensor == senC then
                            bad_sensor = switch_obj.state and senA or senB
                        else
                            bad_sensor = switch_obj.exit_sensor == senB and senA or senB
                        end
                        if sen == bad_sensor then
                            -- The current train has nothing to do with this
                            -- sensor, power-offing
                            SetPower(false)
                            print("detected activity on bad sensorID, power-offing")
                            emit("BadSensor", sid, sen)
                        end
                    end
                end
            elseif state then
                -- Rising edge on any sensor
                if sen == senA then
                    -- (A) -> (C)
                    switch_obj:Lock(false, senC)
                    emit("EnterA", sid)
                elseif sen == senB then
                    -- (B) -> (C)
                    switch_obj:Lock(true, senC)
                    emit("EnterB", sid)
                else
                    -- (C) -> (A|B)
                    emit("EnterC", sid)

                    -- Then lock it to the current state
                    switch_obj:Lock(switch_obj.state, switch_obj.state and senB or senA)
                end
            end
        end
    end

    -- Destroys a SmartSwitch
    -- The object should not be used afterward
    function switch_obj:Disable()
        emit("Disable", sid, state, exit)
        handlers[sid] = nil
    end

    -- Return the switch object
    self.objects[sid] = switch_obj
    return switch_obj
end
