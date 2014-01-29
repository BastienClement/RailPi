--
-- Loading helper
--
function load(file)
	local f, e = loadfile(file)
	if f then
		f()
	else
		print("Error loading file: " .. e)
	end
end
