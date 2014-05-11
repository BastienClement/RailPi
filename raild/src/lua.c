#include "raild.h"
#include <luajit-2.0/lua.h>
#include <luajit-2.0/lualib.h>
#include <luajit-2.0/lauxlib.h>

/**
 * Everything Lua-related
 *
 * Lua is used as an embedded scripting language to control the circuit logic.
 *
 * The C-side of Raild handles everything related to low-level communication
 * with RailHub and TCP/IP API clients and emits events to Lua code to notify
 * changes. Lua scripts can then act accordingly.
 */

// Black-magic for the embedded stdlib.lua file
// This Lua script is embbeded inside the Raild executable file at compile time
extern char _binary_src_stdlib_lua_start; // Pointer to the first caracter of the Lua script
extern char _binary_src_stdlib_lua_size;  // This variable address is the length of the script

// Declare a new Lua CFunction to be called from Lua code
#define API_DECL(name) static int lualib_##name(lua_State *L)

// Creates the library entry for loading every functions into Lua state
#define API_LINK(name) { #name, lualib_##name }

// The Lua VM object
lua_State *L;

/**
 * Safely run a Lua function
 */
static int call(int nargs, int nres) {
    if(lua_pcall(L, nargs, nres, 0) != 0) {
        logger_error(lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

/**
 * Wrapper around call() for event dispatching
 */
static void dispatch(int nargs) {
    call(nargs + 1, 0);
}

/**
 * Prepares to dispatch an internal event
 */
static void prepare_event_internal(const char *ev) {
    // Fetch the event function from bindings
    lua_getfield(L, LUA_REGISTRYINDEX, ev);
}

/**
 * Prepares an event for dispatching
 */
static void prepare_event(const char *ev) {
    // Fetch the event dispatch handler
    prepare_event_internal("DispatchEvent");

    // Push the event name as first argument
    lua_pushstring(L, ev);
}

/**
 * Setup everything Lua-related
 */
void setup_lua(const char *main) {
    // Setup the lua engine
    logger("LUA", "Init Lua engine");
    L = luaL_newstate();

    // Open standards libraries
    luaL_openlibs(L);

    // Load the Raild API
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lualib_register(L);
    lua_pop(L, 1);

    // Black-magic
    // This load the embedded stdlib.lua file
    // As previously noted, the address (and not the value) of the ..._size
    // variable is the length of the embedded script file.
    if(luaL_loadbuffer(L, &_binary_src_stdlib_lua_start, (size_t) &_binary_src_stdlib_lua_size, "stdlib") != 0) {
        logger_light(logger_prefix("load:", lua_tostring(L, -1)));
        exit(1);
    }
    call(0, 0);

    // Alloc the main context
    lua_alloc_context(0, "INTERNAL");

    // Load the main script if provided
    if(main) {
        logger("LUA", logger_prefix("Loading local script:", main));

        // Load and run
        if(luaL_loadfile(L, main) != 0 || lua_pcall(L, 0, 0, 0) != 0) {
            logger_light(logger_prefix("load:", lua_tostring(L, -1)));
            exit(1);
        }
    }
}

/**
 * Runs a specific buffer of Lua code
 * Called from the TCP/IP API to run client code
 */
void lua_eval(const char *buffer, size_t length) {
    if(luaL_loadbuffer(L, buffer, length, "API") != 0) {
        logger_error(logger_prefix("Error loading API code:", lua_tostring(L, -1)));
        lua_pop(L, 1);
        return;
    }

    call(0, 0);
}

/**
 * Timer handler
 */
void lua_handle_timer(raild_event *event) {
    // Fetch the function associated with this timer
    lua_pushlightuserdata(L, event);
    lua_gettable(L, LUA_REGISTRYINDEX);

    // Call it
    call(0, 0);
}

//---------------------------------------------------------------------------//
// Lua public events
//---------------------------------------------------------------------------//

/**
 * Ready event
 * Fired ever time the RailHub sends a READY opcode
 */
void lua_onready() {
    prepare_event("Ready");
    dispatch(0);
}

/**
 * Disconnect event
 * Fired when RailHub failed to send keep-alive opcode and
 * is now considered disconnected
 */
void lua_ondisconnect() {
    prepare_event("Disconnect");
    dispatch(0);
}

/**
 * Sensor changed event
 * Fired when any of the 24 sensors changes state
 */
void lua_onsensorchanged(int sensorid, bool state) {
    prepare_event("SensorChanged");
    lua_pushnumber(L, sensorid);
    lua_pushboolean(L, state);
    dispatch(2);
}

/**
 * Switch changed event
 * Fired when any of the 8 switches changes state
 */
void lua_onswitchchanged(int switchid, bool state) {
    prepare_event("SwitchChanged");
    lua_pushnumber(L, switchid);
    lua_pushboolean(L, state);
    dispatch(2);
}

/**
 * Init event
 */
void lua_oninit() {
    prepare_event("Init");
    dispatch(0);
}

//---------------------------------------------------------------------------//
// Lua internal events
//---------------------------------------------------------------------------//

/**
 * Context allocated event
 */
void lua_alloc_context(int fd, const char *cls) {
    prepare_event_internal("AllocContext");
    lua_pushnumber(L, fd);
    lua_pushstring(L, cls);
    call(2, 0);
}

/**
 * Context deallocated event
 */
void lua_dealloc_context(int fd) {
    prepare_event_internal("DeallocContext");
    lua_pushnumber(L, fd);
    call(1, 0);
}

/**
 * Sets the current script context
 */
void lua_switch_context(int fd) {
    prepare_event_internal("SwitchContext");
    lua_pushnumber(L, fd);
    call(1, 0);
}

/**
 * Removes the script context
 */
void lua_restore_context() {
    prepare_event_internal("RestoreCtx");
    call(0, 0);
}

/**
 * Timer auto-deletec event
 */
void lua_delete_timer(void *timer) {
    prepare_event_internal("DeleteTimer");
    lua_pushlightuserdata(L, timer);
    call(1, 0);
}

//---------------------------------------------------------------------------//
// LUA C API
// C functions to be called by Lua code
//---------------------------------------------------------------------------//

/**
 * exit()
 * A way for Lua scripts to kill the whole process
 */
API_DECL(exit) {
    logger("LUA", "Script killed the main process!");
    exit(2);
}

/**
 * HubReady()
 * Returns the readiness of RailHub
 */
API_DECL(IsHubReady) {
    lua_pushboolean(L, get_hub_readiness());
    return 1;
}

/**
 * SetPower()
 * Set the power state of the circuit
 */
API_DECL(SetPower) {
    set_power(lua_toboolean(L, 1));
    return 0;
}

/**
 * IsPowered()
 * Returns the circuit power state
 */
API_DECL(IsPowered) {
    lua_pushboolean(L, get_power());
    return 1;
}

/**
 * GetSwitch(switch_id)
 * Return the cached state for the requested switch
 */
API_DECL(GetSwitch) {
    int sid = luaL_checknumber(L, 1);
    if(sid < 1 || sid > 8) {
        luaL_error(L, "out of bounds switch id");
    }

    lua_pushboolean(L, get_hub_state(RHUB_SWITCHES) & (1 << (sid - 1)));
    return 1;
}

/**
 * SetSwitch(switch_id, new_state)
 * Request a switch state modification
 */
API_DECL(SetSwitch) {
    int  sid   = luaL_checknumber(L, 1);
    bool state = lua_toboolean(L, 2);

    if(sid < 1 || sid > 8) {
        luaL_error(L, "out of bounds switch id");
    }

    if(state) {
        uart_setswitch_on(sid - 1);
    } else {
        uart_setswitch_off(sid - 1);
    }

    return 0;
}

/**
 * GetSensor(sensor_id)
 * Return the cached state for the requested sensor
 */
API_DECL(GetSensor) {
    int sid = luaL_checknumber(L, 1);

    if(sid < 1 || sid > 24) {
        luaL_error(L, "out of bounds switch id");
    } else if(sid < 9) {
        lua_pushboolean(L, get_hub_state(RHUB_SENSORS1) & (1 << (sid - 1)));
    } else if(sid < 17) {
        lua_pushboolean(L, get_hub_state(RHUB_SENSORS2) & (1 << (sid - 9)));
    } else {
        lua_pushboolean(L, get_hub_state(RHUB_SENSORS3) & (1 << (sid - 17)));
    }

    return 1;
}

//---------------------------------------------------------------------------//
// LUA Private API
// C functions to be wrapped and then called by internal Lua code
//---------------------------------------------------------------------------//

/**
 * __rd_bind(binding, handler)
 * Declare internal event binding
 */
API_DECL(__rd_bind) {
    // Binding name
    const char* binding = luaL_checkstring(L, 1);

    // Function handler
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue (L, 2);

    // Set it in registry
    lua_setfield(L, LUA_REGISTRYINDEX, binding);
    return 0;
}

/**
 * __rd_send(data)
 * Sends a data string to the remote point of the API
 */
API_DECL(__rd_send) {
    luaL_checkstring(L, 1);
    int fd = luaL_checknumber(L, 2);

    size_t len;
    const char *buffer = lua_tolstring(L, 1, &len);
    write(fd, buffer, len);

    return 0;
}

/**
 * __rd_create_timer(initial, interval, fn)
 * Creates a new timer ticking after 'initial' millisecond and the
 * every 'interval' millisecond until canceled
 *
 * If interval == 0, this is a one-time timer automatically canceled
 * after its first tick
 *
 * This function is then wrapped by the StdLib to add context switching
 * support.
 */
API_DECL(__rd_create_timer) {
    // Check arguments
    int initial  = luaL_checknumber(L, 1);
    int interval = luaL_checknumber(L, 2);
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

/**
 * __rd_cancel_timer(timer)
 * Cancels the timer
 */
API_DECL(__rd_cancel_timer) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    raild_event *event = (raild_event *) lua_touserdata(L, 1);

    // Ensure the timer still exists
    lua_pushvalue(L, 1);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if(lua_isnil(L, -1)) {
        luaL_error(L, "attempt to cancel an already canceled timer");
    }

    // Effectively removes it
    raild_timer_delete(event);

    return 0;
}

/**
 * __rd_unregister_timer(timer)
 * Removes the callback from the internal callbacks table
 */
API_DECL(__rd_unregister_timer) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    raild_event *event = (raild_event *) lua_touserdata(L, 1);

    // Remove the callback function from the registry
    lua_pushlightuserdata(L, event);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    return 0;
}

//---------------------------------------------------------------------------//
// Library object
//---------------------------------------------------------------------------//
luaL_Reg raild_api[] = {
    API_LINK(exit),

    API_LINK(IsHubReady),
    API_LINK(SetPower),
    API_LINK(IsPowered),
    API_LINK(GetSwitch),
    API_LINK(SetSwitch),
    API_LINK(GetSensor),

    API_LINK(__rd_bind),
    API_LINK(__rd_send),
    API_LINK(__rd_create_timer),
    API_LINK(__rd_cancel_timer),
    API_LINK(__rd_unregister_timer),
    { NULL, NULL }
};

/**
 * Register the raild lua library
 */
void lualib_register() {
    luaL_register(L, NULL, raild_api);
}

