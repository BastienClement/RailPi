
local lock_s1 = false
local lock_s2 = false

function do_lock_s2()
	lock_s2 = true
	CreateTimer(500, 0, function() lock_s2 = false end)
end

On("SensorChanged", function(sid, state)
	print("Sensor", sid, state)
	
	if not lock_s1 then
		if state and sid == 17 then
			SetSwitch(1, not GetSwitch(1))
			lock_s1 = true
			CreateTimer(500, 0, function() lock_s1 = false end)
		end
	end
	
	if not lock_s2 then
		if state and sid == 2 then
			SetSwitch(2, true)
			do_lock_s2()
		elseif state and sid == 10 then
			SetSwitch(2, false)
			do_lock_s2()
		end
	end
end)
