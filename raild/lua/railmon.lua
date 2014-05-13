--
-- Binding points for RailMon client
--
RailMon = EventEmitter()

-- Shortcut to reduce typing over telnet
function RailMon:Bind()
    RailMon:On("JSON", function(json)
        send(json, "\n")
    end)
end

local function emit(event, obj)
    if not obj then obj = {} end
    obj.event = event
    RailMon:Emit("JSON", JSON:Encode(obj))
end

-------------------------------------------------------------------------------
-- Bindings to global events
-------------------------------------------------------------------------------
On("Ready", function()
    emit("Ready")
end)

On("Disconnect", function()
    emit("Disconnect")
end)

On("SensorChange", function(i, s)
    emit("SensorChange", { id = i, state = s })
end)

On("SwitchChange", function(i, s)
    emit("SwitchChange", { id = i, state = s })
end)

-------------------------------------------------------------------------------
-- Bindings to Switches
-------------------------------------------------------------------------------
Switch:On("Lock", function(i)
    emit("SwitchLock", { id = i })
end)

Switch:On("Unlock", function(i)
    emit("SwitchUnlock", { id = i })
end)
