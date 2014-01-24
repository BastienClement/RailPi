--
-- Loading helper
--
local function load(file)
	local f, e = loadfile("lua/" .. file)
	if f then
		f()
	else
		print("Error loading file: " .. e)
	end
end

load "main.lua"
