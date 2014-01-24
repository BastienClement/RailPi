#include "raild.h"

#define API_DECL(name) static int lualib_##name(lua_State *L)
#define API_LINK(name) { #name, lualib_##name }

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
API_DECL(HubReady) {
	lua_pushboolean(L, get_hub_readiness());
	return 1;
}

//
// Library object
//
luaL_Reg raild_api[] = {
	API_LINK(exit),
	API_LINK(HubReady),
	{ NULL, NULL }
};
