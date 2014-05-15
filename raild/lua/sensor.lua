--
-- High-level sensors with built-in debounce
--
Sensor = EventEmitter({
    -- Debounce delay (ms)
    debounce = 25
})

-- Handlers list
-- Enabled sensor will list themselve here
local handlers = {}

-- Bind the event handler
On("SensorChange", function(id, state)
    if handlers[id] then
        handlers[id](state)
    end
end)

-- Create one object for each sensor
do
    local function create_sensor(id)
        -- The sensor object
        local this = EventEmitter({
            ["id"] = id,
            ["state"] = false,
            ["enabled"] = false
        })

        -- Emit events on both the sensor itself and the global Sensor object
        local function emit(event, ...)
            this:Emit(event, ...)
            Sensor:Emit(event, this, ...)
        end

        local function do_change(state)
            this.state = state
            emit("Change", state)
        end

        -- Debounce sensor state change
        local delay
        local function handler(state)
            if delay then CancelTimer(delay) end
            if this.state ~= state then
                local debounce = Sensor.debounce
                if debounce > 0 then
                    delay = CreateTimer(debounce, 0, function()
                        do_change(state)
                        delay = nil
                    end)
                else
                    do_change(state)
                end
            end
        end

        -- Enable this sensor
        function this:Enable()
            if self.enabled then return else self.enabled = true end
            self.state = GetSensor(id)
            handlers[id] = handler
            emit("Enable")
        end

        -- Disable this sensor
        function this:Disable()
            if self.enabled then self.enabled = false else return end
            handlers[id] = nil
            emit("Disable")
        end

        return this
    end

    for i = 1, 24 do Sensor[i] = create_sensor(i) end
end

-- Enable multiple sensors at once
function Sensor:Enable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]]:Enable()
    end
end

-- Disable multiple sensors at once
function Sensor:Disable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]]:Disable()
    end
end
