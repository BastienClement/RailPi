function onready()
	print "ready"
end

function ontick()
	print "tick";
end

local i = 0
local t
t = TimerCreate(1000,1000, function()
	i = i + 1
	if i > 5 then
		print("done")
		TimerCancel(t)
	else
		print("tick: " .. i)
	end
end)

