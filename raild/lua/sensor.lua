--
-- High-level sensors with built-in debounce
--
Sensor = EventEmitter({
    -- Debounce delay (ms)
    debounce = 25
})

local sensor_mt = { __tostring = function() return "[object Sensor]" end }

-- Handlers list
-- Enabled sensor will list themselve here
local handlers = {}

-- Bind the event handler
On("SensorChange", function(id, state)
    if handlers[id] then
        handlers[id](state)
    end
end)

-- Create sensors lazily
setmetatable(Sensor, {
    __index = function(_, id)
        if type(id) ~= "number"
        or id < 1 or id > 24 then
            error("invalid sensor id: " .. id)
        end

        -- The sensor object
        local self = EventEmitter()
        local state = false
        local enabled = false

        -- Accessors
        function self.GetState() return state end
        function self.GetId() return id end
        function self.IsEnabled() return enabled end

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
            -- Cancel previous timer
            if delay then
                CancelTimer(delay)
                delay = nil
            end

            -- Update state if needed
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
            return self
        end

        -- Disable this sensor
        function self.Disable()
            if enabled then enabled = false else return end
            handlers[id] = nil
            emit("Disable")
            return self
        end

        -- Register and enable this sensor
        Sensor[id] = self
        self.Enable()

        return setmetatable(self, sensor_mt)
    end
})

-- Enable multiple sensors at once
function Sensor.Enable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]].Enable()
    end
    return Sensor
end

-- Disable multiple sensors at once
function Sensor.Disable(...)
    local sensors = {...}
    for i = 1, #sensors do
        Sensor[sensors[i]].Disable()
    end
    return Sensor
end

-- Check if an object is a sensor object
function Sensor.IsSensor(obj)
    return getmetatable(obj) == sensor_mt
end
