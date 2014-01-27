#include "raild.h"
#include <luajit-2.0/lua.h>
#include <luajit-2.0/lualib.h>
#include <luajit-2.0/lauxlib.h>

extern char _binary_src_stdlib_lua_start;
extern char _binary_src_stdlib_lua_size;

#define API_DECL(name) static int lualib_##name(lua_State *L)
#define API_LINK(name) { #name, lualib_##name }

// The Lua VM
lua_State *L;

//
// Safely run a Lua function
//
static int call(int nargs, int nres) {
	if(lua_pcall(L, nargs, nres, 0) != 0) {
		printf("error: %s\n", lua_tostring(L, -1));
		return 0;
	}

	return 1;
}

//
// Setup everything Lua-related
//
void setup_lua(const char *main) {
	// Setup the lua engine
	printf("[LUA]\t Init Lua engine\n");
	L = luaL_newstate();

	// Open standards libraries
	luaL_openlibs(L);

	// Load the Raild API
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lualib_register(L);
	lua_pop(L, 1);

	// TODO: comment
	luaL_loadbuffer(L, &_binary_src_stdlib_lua_start, (size_t) &_binary_src_stdlib_lua_size, "stdlib.lua");
	lua_pcall(L, 0, 0, 0);

	// Load the main script if provided
	if(main) {
		printf("[LUA]\t Loading local script: %s\n", main);

		// Load and run
		if(luaL_loadfile(L, main) != 0 || lua_pcall(L, 0, 0, 0) != 0) {
			printf("load: %s\n", lua_tostring(L, -1));
			exit(1);
		}
	}
}

//
// Prepares an event for dispatching
//
static int event_prepare(const char *ev) {
	lua_getfield(L, LUA_GLOBALSINDEX, ev);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	return 1;
}

//
// Ready event
//
int lua_onready() {
	if(!event_prepare("onready")) return 0;
	return call(0, 0);
}


void lua_handle_timer(raild_event *event) {
	lua_pushlightuserdata(L, event);
	lua_gettable(L, LUA_REGISTRYINDEX);
	call(0, 0);
}

//
// --- LUA C API ---
//

//
// exit()
//
API_DECL(exit) {
	printf("[LUA]\t Script killed the main process!\n");
	exit(2);
}

//
// HubReady()
//
API_DECL(HubReady) {
	lua_pushboolean(L, get_hub_readiness());
	return 1;
}

//
// TimerCreate(initial, interval, fn)
//
// Creates a new timer
//
API_DECL(TimerCreate) {
	// Check arguments
	int initial  = (int) luaL_checknumber(L, 1);
	int interval = (int) luaL_checknumber(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);

	// Create the timer
	raild_event *event = raild_timer_create(initial, interval, RAILD_EV_LUA_TIMER);

	// Store the callback function
	lua_pushlightuserdata(L, event);
	lua_pushvalue(L, 3);
	lua_settable(L, LUA_REGISTRYINDEX);

	// Return the timer event object as light user data
	lua_pushlightuserdata(L, event);
	return 1;
}

//
// TimerCancel(timer)
//
// Cancels the timer
//
API_DECL(TimerCancel) {
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	raild_event *event = (raild_event *) lua_touserdata(L, 1);

	// Ensure the timer still exists
	lua_pushlightuserdata(L, event);
	lua_gettable(L, LUA_REGISTRYINDEX);
	if(lua_isnil(L, -1)) {
		luaL_error(L, "attempt to cancel an already canceled timer");
	}

	// Effectively removes it
	raild_timer_delete(event);

	// Remove the callback function from the registry
	lua_pushlightuserdata(L, event);
	lua_pushnil(L);
	lua_settable(L, LUA_REGISTRYINDEX);

	return 0;
}

//
// Library object
//
luaL_Reg raild_api[] = {
	API_LINK(exit),
	API_LINK(HubReady),
	API_LINK(TimerCreate),
	API_LINK(TimerCancel),
	{ NULL, NULL }
};

//
// Register the raild lua library
//
void lualib_register() {
	luaL_register(L, NULL, raild_api);
}

