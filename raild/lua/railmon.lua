--
-- Binding point for RailMon client
--
RailMonEvents = EventEmitter()

local function emit(event, obj)
    obj.event = event
    RailMonEvents:Emit("JSON", JSON:Encode(obj))
end

On("Ready", function() emit("Ready", {}) end)
On("Disconnect", function() emit("Disconnect", {}) end)

On("SensorChange", function(i, s)
    emit("SensorChange", { id = i, state = s })
end)

On("SwitchChange", function(i, s)
    emit("SwitchChange", { id = i, state = s })
end)
