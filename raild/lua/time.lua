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
