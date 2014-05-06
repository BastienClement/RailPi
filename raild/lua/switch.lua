-- List of switches defined
local switches = {}

-- Bind the event handler on first CreateAutoSwitch() call
local DoBind
do
    local bound = false
    local function DoBind()
        if bound then return else bound = true end
        On("SensorChanged", function(sid, state)
            for sid, switch in pairs(switches) do
                switch:handleEvent(sid, state)
            end
        end)
    end
end

-- Creates a new auto-managed switch object
--      (1)  (2)
--      | | / /
--      | |/ /     STATE      EXIT
--      |/| /      false      (1)
--      | |/       true       (2)
--      | |
--      (3)
function CreateAutoSwitch(sid, sen1, sen2, sen3)
    -- Bind event handler if not already done
    DoBind()

    -- Switch object
    local switch = {
        ["locked"] = false,
        ["locked_source"] = -1,
        ["locked_state"] = false,
        ["state"] = false,
        ["handler"] = nil
    }

    -- Lock this switch in a given position
    function switch:lock(state, source)
        self.locked = true
        self.locked_state = state
        self.locked_source = source
    end

    -- Free the previously set lock
    function switch:unlock(state)
        self.locked = false
        self.locked_source = -1
        if self.locked_state ~= self.state then
            self:set(self.state)
        end
    end

    -- Set the switch in a given position
    -- If the switch is locked, does nothing
    function switch:set(state)
        self.state = state
        if not self.locked then
            SetSwitch(sid, self.state)
        end
    end

    -- Define the callback handler when sensor3 is enabled
    function switch:setHandler(fn)
        self.handler = fn
    end

    -- Handle sensor state changes
    function switch:handleEvent(sen, state)
        if self.locked then
            -- Todo
        else
            -- Todo
        end
    end

    -- Register this switch and return it
    switches[sid] = switch
    return switch
end
