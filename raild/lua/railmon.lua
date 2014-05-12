--
-- Binding point for RailMon client
--
RailMon = EventEmitter()

function RailMon:Send(event, obj)
    obj.event = event
    RailMon:Emit("JSON", JSON:Encode(obj))
end

On("Ready", function()
    RailMon:Send("Ready", {})
end)

On("Disconnect", function()
    RailMon:Send("Disconnect", {})
end)

On("SensorChange", function(i, s)
    RailMon:Send("SensorChange", { id = i, state = s })
end)

On("SwitchChange", function(i, s)
    RailMon:Send("SwitchChange", { id = i, state = s })
end)

function RailMon:Bind()
    RailMon:On("JSON", function(json)
        send(json, "\n")
    end)
end
