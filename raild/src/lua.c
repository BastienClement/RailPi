#include "raild.h"

// The Lua VM
static lua_State *L;

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
	luaL_register(L, NULL, raild_api);
	lua_pop(L, 1);

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
