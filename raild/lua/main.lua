function OnReady()
	print "ready"
end

function OnSensorChanged(sid, state)
	print("Sensor", sid, state)
	if state and sid == 1 then
		print("Switch", true)
		SetSwitch(1, false)
	elseif state and (sid == 9 or sid == 17) then
		print("Switch", false)
		SetSwitch(1, true)
	end
end
