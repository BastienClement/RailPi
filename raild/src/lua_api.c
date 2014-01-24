#include "raild.h"

#define API_DECL(name) static int lib_##name(lua_State *L)
#define API_LINK(name) { #name, lib_##name }

//
// exit()
//
API_DECL(exit) {
	printf("[LUA]\t Script killed the main process!\n");
	exit(2);
}

//
// tick_interval()
//
API_DECL(tick_interval) {
	int interval = tick_interval;
	tick_interval = (int) lua_tonumber(L, -1);
	lua_pushnumber(L, interval);
	return 1;
}

//
// Library object
//
luaL_Reg raild_api[] = {
	API_LINK(exit),
	API_LINK(tick_interval),
	{ NULL, NULL }
};
