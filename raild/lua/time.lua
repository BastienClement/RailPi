--
-- Get current time
--
local ffi = require("ffi")
ffi.cdef[[
    typedef long time_t;
    typedef struct timeval {
        time_t tv_sec;
        time_t tv_usec;
    } timeval;

    int gettimeofday(struct timeval* t, void* tzp);
]]

local C = ffi.C
local now = ffi.new("struct timeval")

function Time()
    C.gettimeofday(now, nil)
    return now.tv_sec + (now.tv_usec / 1000000)
end

--
-- Chrono helper
--
function Chrono(window)
    window = window or 5

    local self = {}
    local start = Time()
    local history = {}
    local queue = {}
    local cache_mean = nil

    function self.Reset()
        start = Time()
    end

    function self.Time()
        return Time() - start
    end

    function self.Push()
        table.insert(queue, Time())
    end

    function self.Shift()
        if #queue < 1 then return 0/0 end
        local t = Time() - table.remove(queue, 1)
        table.insert(history, t)
        cache_mean = nil
        if #history > window then
            table.remove(history, 1)
        end
        return t
    end

    function self.Mean()
        if cache_mean then return cache_mean end
        local sum = 0
        for i = 1, #history do sum = sum + history[i] end
        cache_mean = sum / #history
        return cache_mean
    end

    return self
end
