JSON = {}

-- Null value
JSON.null = {}

-- Mark table as object
local object_mt = { __tostring = function() return "JSON Object" end }
function JSON:Object(table)
    return setmetatable(table, object_mt)
end

-- Mark table as array
local array_mt = { __tostring = function() return "JSON Array" end }
function JSON:Array(table)
    return setmetatable(table, array_mt)
end

--
-- JSON:Encode
--
do
    local function encodeNumber(n)
        -- TODO: not JSON-safe
        return tostring(n)
    end

    local function encodeString(s)
        -- TODO: not JSON-safe
        return "\"" .. tostring(s) .. "\""
    end

    local function encodeObject(b, o)
        for key, value in pairs(o) do
            b[#b+1] = encodeString(key) .. ":" .. JSON:Encode(value)
        end
        return "{", "}"
    end

    local function encodeArray(b, a)
        for index, value in ipairs(a) do
            b[#b+1] = JSON:Encode(value)
        end
        return "[", "]"
    end

    function JSON:Encode(obj)
        local t = type(obj)
        print(t)
        if t == "nil" then
            return "null"
        elseif t == "boolean" then
            return obj and "true" or "false"
        elseif t == "number" then
            return encodeNumber(obj)
        elseif t == "string" then
            return encodeString(obj)
        elseif t == "table" then
            -- Null value placeholder is an table
            if obj == JSON.null then
                return "null"
            end

            local b = {}  -- Entries buffer
            local dl, dr  -- Left/right delimiters

            -- Guess if this table is an object or an array
            if tostring(obj) == "JSON Object" or #obj == 0 then
                dl, dr = encodeObject(b, obj)
            else
                dl, dr = encodeArray(b, obj)
            end

            return dl .. table.concat(b, ",") .. dr
        else
            -- Unknown type (function / userdata / thread)
            error("cannot JSON-encode value from type: " .. obj)
        end
    end
end
