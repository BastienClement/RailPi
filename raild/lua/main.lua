function OnReady()
	print "ready"
end

function OnSensorChanged(sid, state)
	print("Sensor", sid, state)
	
	if state and sid == 17 then
		SetSwitch(1, not GetSwitch(1))
	end
	
	if state and sid == 2 then
		SetSwitch(2, true)
	elseif state and sid == 10 then
		SetSwitch(2, false)
	end
end
