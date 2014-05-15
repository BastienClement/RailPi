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
        local self = EventEmitter()

        local state = false
        local enabled = false

        -- Emit events on both the sensor itself and the global Sensor object
        local function emit(event, ...)
            self.Emit(event, ...)
            Sensor.Emit(event, self, ...)
        end

        -- Change cached state and emit event
        local function update(new_state)
            state = new_state
            emit("Change", new_state)
        end

        -- Debounce sensor state change
        local delay
        local function handler(new_state)
            if delay then CancelTimer(delay) end
            if state ~= new_state then
                local debounce = Sensor.debounce
                if debounce > 0 then
                    delay = CreateTimer(debounce, 0, function()
                        update(new_state)
                        delay = nil
                    end)
                else
                    update(new_state)
                end
            end
        end

        -- Enable this sensor
        function self.Enable()
            if enabled then return else enabled = true end
            state = GetSensor(id)
            handlers[id] = handler
            emit("Enable")
        end

        -- Disable this sensor
        function self.Disable()
            if enabled then enabled = false else return end
            handlers[id] = nil
            emit("Disable")
        end

        return self
    end

    for i = 1, 24 do Sensor[i] = create_sensor(i) end
end

-- Enable multiple sensors at once
function Sensor.Enable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]].Enable()
    end
end

-- Disable multiple sensors at once
function Sensor.Disable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]].Disable()
    end
end
