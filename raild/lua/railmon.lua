--
-- Binding points for RailMon client
--
RailMon = EventEmitter()

-- Shortcut to reduce typing over telnet
function RailMon.Bind()
    RailMon.On("JSON", function(json)
        send(json, "\n")
    end)
end

-- Local Sync() cache
local sync_cache

-- Emit event object with proper formatting
local function emit(event, obj, keep_cache)
    -- Obj is optional
    if not obj then obj = {} end

    -- Delete cache if not told to keep it
    if not keep_cache then sync_cache = nil end

    -- Add event name and emit JSON
    obj.event = event
    RailMon.Emit("JSON", JSON.Encode(obj))
end

-- Get full circuit state for RailMon
function RailMon.Sync()
    -- Check if cache is available
    if sync_cache then emit("Sync", sync_cache, true) end

    local _sensors = {}
    for i = 1, 24 do
        _sensors[i] = Sensors[i].GetState()
    end

    local _switches = {}
    local _locks = {}
    for i = 1, 8 do
        local switch = Switches[i]
        _switches[i] = switch:GetState()
        _locks[i] = switch:IsLocked()
    end

    -- Build cache
    sync_cache = {
        switches = _switches,
        sensors = _sensors,
        locks = _locks,
        ready = IsHubReady(),
        power = IsPowered()
    }

    emit("Sync", sync_cache, true)
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

On("Power", function(state)
    emit("Power", { state = state })
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
Switches:On("Lock", function(switch)
    emit("SwitchLock", { id = switch.GetId() })
end)

Switches:On("Unlock", function(switch)
    emit("SwitchUnlock", { id = switch.GetId() })
end)
