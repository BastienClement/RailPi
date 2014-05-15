--
-- Auto-managed switches
--
Switch = EventEmitter({
    handlers = {}
})

-- Bind the event handler on first Swtich:Create() call
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
    local this = EventEmitter({
        ["id"] = sid,
        ["locked"] = false,
        ["exit_sensor"] = -1,
        ["enter_sensor"] = -1,
        ["state"] = false
    })

    -- Emit events on both the switch object and the global
    local function emit(event, ...)
        this:Emit(event, ...)
        Switch:Emit(event, this, ...)
    end

    -- Lock this switch in a given position
    function this:Lock(state)
        self:Set(state)
        self.locked = true
        emit("Lock", state)
    end

    -- Free the previously set lock
    function this:Unlock()
        self.locked = false
        self.enter_sensor = -1
        self.exit_sensor = -1
        emit("Unlock")
    end

    -- Set the switch in a given position
    -- If the switch is locked, does nothing
    function this:Set(state, silent)
        -- Ignore call if switch is already is this position
        if self.state == state then return end

        -- Update internal state
        self.state = state

        -- If not locked, then send to RailHub
        if not self.locked then
            SetSwitch(sid, self.state)
            emit("Set", state)
        elseif not silent then
            error("attempt to set or lock an already locked switch")
        end
    end

    -- Toggle switch state
    function this:Toggle(silent)
        self:Set(not self.state, silent)
    end

    -- Register this switch and its event handler
    local delayTimer
    handlers[sid] = function(sen, state)
        -- Only react to related sensors
        if sen == senA
        or sen == senB
        or sen == senC then
            if this.locked then
                if sen == this.exit_sensor then
                    -- Falling or rising edge on exit sensor
                    if not state then
                        -- Falling edge, delay unlock by 100ms
                        delayTimer = CreateTimer(100, 0, function()
                            this:Unlock()
                        end)
                    else
                        -- Rising edge, cancel the unlock delay
                        CancelTimer(delayTimer)
                    end
                else
                    if state then
                        if sen ~= this.enter_sensor
                        and sen ~= this.exit_sensor then
                            -- The current train has nothing to do with this
                            -- sensor, power-offing
                            SetPower(false)
                            print("detected activity on bad sensorID, power-offing")
                            emit("BadSensor", sen)
                        end
                    end
                end
            elseif state then
                -- Rising edge on any sensor
                if sen == senA then
                    -- (A) -> (C)
                    this.enter_sensor = senA
                    this.exit_sensor = senC
                    this:Lock(false, senC)
                    emit("EnterA")
                elseif sen == senB then
                    -- (B) -> (C)
                    this.enter_sensor = senB
                    this.exit_sensor = senC
                    this:Lock(true, senC)
                    emit("EnterB")
                else
                    -- (C) -> (A|B)
                    emit("EnterC")
                    this.enter_sensor = senC
                    this.exit_sensor = this.state and senB or senA

                    -- Lock it to the current state
                    this:Lock(this.state)
                end
            end
        end
    end

    -- Destroys a SmartSwitch
    -- The object should not be used afterward
    function this:Disable()
        emit("Disable", sid, state, exit)
        handlers[sid] = nil
    end

    -- Return the new switch object
    self[sid] = this
    return this
end
