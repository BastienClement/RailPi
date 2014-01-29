function OnReady()
	print "ready"
end

function OnSensorChanged(sid, state)
	SetSwitch(sid, state)
end