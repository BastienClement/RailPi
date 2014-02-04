-- Private StdLib functions
local SwitchCtx, RestoreCtx

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

--
-- Output wrapper
--
do
	-- Alias the original print() as trace()
	trace = print
	
	-- Print is now context-aware and send() data
	-- to API client when called from an API context
	function print(...)
		if CtxClass(GetCtx()) ~= "API_CLIENT" then
			-- This is not an API context
			trace(...)
		else
			-- List of every arguments
			local n = select("#", ...)
			local a = {...}
			
			-- Build the output string part-by-part
			local buffer = {}
			for i = 1, n do
				buffer[#buffer + 1] = tostring(a[i])
			end
			
			-- Concat & send
			send(table.concat(buffer, "\t") .. "\r\n")
		end
	end
end

--
-- Events
--
do
	-- List of registered event handlers
	local events = {}
	
	--
	-- Dispatch event to registered handlers
	--
	function __dispatch_event(event, ...)
		-- This event has nothing registered to it
		if not events[event] then return end
		
		-- Loops over every handlers
		for _, handler in ipairs(events[event]) do
			-- Safe call, error in one handler should not prevent
			-- others to be run correctly
			SwitchCtx(handler.ctx)
			local success, error = pcall(handler.fn, ...)
			RestoreCtx()
			if not success then
				print("[LUA]\t Error while dispatching event: " .. error)
			end
		end
	end
	
	--
	-- Attaches a new handler to an event
	--
	function On(event, fn)
		-- First time an event is registered
		if not events[event] then
			events[event] = {}
		end
		
		-- Adds this function to the event handler
		-- table along with context informations
		table.insert(events[event], {
			ctx = GetCtx(),
			fn  = fn
		})
	end
	
	--
	-- Detach a function previously registered with On
	--
	-- If the fn argument is nil, disables every handlers
	-- registered from the current context on this event
	--
	function Off(event, fn)
		-- Nothing registered to this event, so obviously
		-- nothing to removes...
		if not events[event] then return end
		
		-- Current script context
		local ctx = GetCtx();
		
		-- Check matching handler
		for idx, handler in ipairs(events[event]) do
			if fn then
				if handler.fn == fn then
					table.remove(events[event], idx)
				end
			elseif handler.ctx == ctx then
				table.remove(events[event], idx)
			end
		end
	end
	
	--
	-- Function called when a file descriptor is deallocated
	--
	-- This functions checks every handlers and removes
	-- those registered from this context
	--
	On("CtxDealloc", function(ctx)
		for _, handlers in pairs(events) do
			for idx, handler in ipairs(handlers) do
				if handler.ctx == ctx then
					table.remove(handlers, idx)
				end
			end 
		end
	end)
end

--
-- Contexts manager
--
do
	-- void set_ctx(ctx)
	local set_ctx = __rd_set_ctx
	__rd_set_ctx = nil
	
	-- List of every context currently available
	local ctxs = {}
	
	-- Tracks context allocations
	On("CtxAlloc", function(ctx, class)
		ctxs[ctx] = class
	end)
	
	-- Tracks context deallocations
	On("CtxDealloc", function(ctx)
		ctxs[ctx] = nil
	end)
	
	-- Returns the class of a given context
	function CtxClass(ctx)
		if ctx == 0 then return "INTERNAL" end
		return ctxs[ctx] or "UNKNOWN"
	end
	
	-- Checks if a given context class is script-allowed
	-- Only API-Client and Internal are allowed contexts
	local classes_whitelist = { ["API_CLIENT"] = true, ["INTERNAL"] = true }
	local function IsCtxAllowed(ctx)
		return classes_whitelist[CtxClass(ctx)] or false
	end
	
	--
	-- Context stack management
	--
	
	-- The context stack
	-- Every context switch push the old context on it and
	-- context restores pops its value from it
	local ctxStack = {}
	
	-- Switch to a new context and store the old one on the stack
	function SwitchCtx(new_ctx)
		-- Checks that the targeted context is allowed as a
		-- script context
		if IsCtxAllowed(new_ctx) then
			ctxStack[#ctxStack + 1] = GetCtx()
			set_ctx(new_ctx)
		else
			error("attempted to switch to a forbidden script context")
		end
	end
	
	-- Restore the old context from the stack
	function RestoreCtx()
		local top = #ctxStack
		
		-- Checks that the stack is not empty
		if top > 0 then
			local ctx = ctxStack[top]
			set_ctx(ctx)
			table.remove(ctxStack, top)
		end
	end
end

--
-- Timers
--
do
	-- tid create_timer(initial, interval, fn)
	-- Creates a new timer firing after `initial` milliseconds
	-- and then every `interval` milliseconds, running `fn`
	local create_timer = __rd_create_timer
	__rd_create_timer = nil
	
	-- void cancel_timer(tid)
	-- Cancel a timer from a user-data returned by create_timer()
	local cancel_timer = __rd_cancel_timer
	__rd_cancel_timer = nil
	
	-- void unregister_timer(tid)
	-- Unregister the callback function
	local unregister_timer = __rd_unregister_timer
	__rd_unregister_timer = nil
	
	-- List of all defined timers
	local timers = {}
	
	-- Create a new timer that will fire for the first time after `initial`
	-- milliseconds and then every `interval` milliseconds, calling the
	-- callback function `fn`
	function CreateTimer(initial, interval, fn)
		-- Current context
		local ctx = GetCtx()
		
		-- Create the timer
		local tid = create_timer(initial, interval, function()
			-- Enter context before execution
			SwitchCtx(ctx)
			
			-- Exec the timer callback function
			local success, error = pcall(fn)
			if not success then
				print(error)
			end
			
			-- Restore old context after execution
			RestoreCtx()
		end)
		
		-- Save the context of this timer
		timers[tid] = ctx
		
		return tid
	end
	
	-- Cancel a not-yet-fired timer
	function CancelTimer(tid)
		if not timers[tid] then return end
		timers[tid] = nil     -- deletes the Lua reference
		cancel_timer(tid)     -- cancels the timer
		unregister_timer(tid) -- unregisters the callback
	end
	
	-- Cleanup after automatic collection of one-time timers
	On("TimerDeleted", function(tid)
		if not timers[tid] then return end
		timers[tid] = nil     -- deletes the Lua reference
		unregister_timer(tid) -- unregisters the callback
		-- There is no need to cancel the timer as this is
		-- an auto-collected timer already expired.
	end)
	
	-- Tracks context deallocations and cancel timers associated
	-- with these contexts
	On("CtxDealloc", function(ctx)
		for tid, tctx in pairs(timers) do
			if tctx == ctx then
				CancelTimer(tid)
			end
		end
	end)
end
