--
-- Auto-managed switches
--
Switch = EventEmitter({
    -- Delay before unlocking the switch (ms)
    debounce = 100
})

local switch_mt = { __tostring = function() return "[object Switch]" end }

-- Lazy sensor creation
--
--      (A) (B)
--      | |/ /
--      | / /     STATE      EXIT
--      |/ /      false      (A)
--      | /       true       (B)
--      | |
--      (C)
--
setmetatable(Switch, {
    __index = function(_, id)
        if type(id) ~= "number"
        or id < 1 or id > 8 then
            error("invalid switch id: " .. id)
        end

        -- The switch object
        local self = EventEmitter()
        local enabled = false
        local state = false
        local pending_state = false
        local locked = false

        local senA, senB, senC
        local enter_sensor, exit_sensor
        local delay

        -- Accessors
        function self.GetId() return id end
        function self.GetState() return state end
        function self.IsEnabled() return enabled end
        function self.IsLocked() return locked end

        -- Emit event on both the switch itself and the global Switch object
        local function emit(event, ...)
            self.Emit(event, ...)
            Sensor.Emit(event, self, ...)
        end

        -- Unlock this switch
        local function unlock()
            locked = false
            enter_sensor = nil
            exit_sensor = nil
            emit("Unlock")
            self.Set(pending_state)
        end

        -- Handle sensor events
        local function handler(sen, rising)
            -- Only react to related sensors
            if sen == senA
            or sen == senB
            or sen == senC then
                if locked then
                    if sen == exit_sensor then
                        -- Falling or rising edge on exit sensor
                        if rising then
                            -- Rising edge, cancel the unlock delay
                            if delay then
                                CancelTimer(delay)
                                delay = nil
                            end
                        else
                            -- Falling edge, delay unlock
                            delay = CreateTimer(Switch.debounce, 0, unlock)
                        end
                    else
                        -- Activity on another sensor
                        if  rising
                        and sen ~= enter_sensor
                        and sen ~= exit_sensor then
                            -- The current train has nothing to do with this
                            -- sensor, power-offing
                            SetPower(false)
                            print("detected activity on bad sensorID, power-offing")
                            emit("BadSensor", sen)
                        end
                    end
                elseif rising then
                    -- Rising edge,  lock this switch
                    if sen == senA then
                        -- (A) -> (C)
                        emit("EnterA")
                        enter_sensor = senA
                        exit_sensor = senC
                        self.Set(false)
                    elseif sen == senB then
                        -- (B) -> (C)
                        emit("EnterB")
                        enter_sensor = senB
                        exit_sensor = senC
                        self.Set(true)
                    else
                        -- (C) -> (A|B)
                        emit("EnterC")
                        enter_sensor = senC
                        exit_sensor = state and senB or senA
                    end

                    locked = true
                    emit("Lock")
                end
            end
        end

        -- Enable this switch and bind it to given sensors
        function self.Enable(a, b, c)
            if enabled then
                error("attempt to enable an already enabled switch")
            end

            -- Cast sensor IDs to sensor objects
            if type(a) == "number" then a = Sensor[a] end
            if type(b) == "number" then b = Sensor[b] end
            if type(c) == "number" then b = Sensor[b] end

            -- Check if arguments are valid sensor objects
            if not Sensor.IsSensor(a)
            or not Sensor.IsSensor(b)
            or not Sensor.IsSensor(c) then
                error("invalid arguments given to switch.Enable()")
            end

            -- Save them
            senA, senB, senC = a, b, c

            -- Sync switch state
            enabled = true
            state = GetSwitch(id)
            pending_state = state
            locked = false

            -- Register handler
            Sensor:On("Change", handler)

            emit("Enable")
            return self
        end

        -- Disable this switch
        function self.Disable()
            -- Remove binding to sensors
            Sensor:Off("Change", handler)

            -- Cleanup potential timer
            if delay then
                CancelTimer(delay)
                delay = nil
            end

            -- Unlock this switch
            unlock()

            enabled = false
            emit("Disable")
            return self
        end

        -- Set the switch in a given position
        function self.SetState(new_state)
            if locked then
                -- Store the state for later
                pending_state = new_state
            elseif state ~= new_state then
                -- Update state
                state = new_state
                pending_state = new_state
                SetSwitch(id, new_state)
                emit("Set", new_state)
            end
            return self
        end

        -- Register and return this switch
        Switch[id] = self
        return setmetatable(self, switch_mt)
    end
})
