#ifndef RAILD_H
#define RAILD_H 1

//---------------------------------------------------------------------------//
// INCLUDES
//---------------------------------------------------------------------------//
// Common libraries
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

//---------------------------------------------------------------------------//
// Types
//---------------------------------------------------------------------------//
// List of ports physically available on RailHub
typedef enum {
    RHUB_SENSORS1,
    RHUB_SENSORS2,
    RHUB_SENSORS3,
    RHUB_SWITCHES
} rhub_port;

// fd type indicator for epoll events
typedef enum {
    RAILD_EV_UART,       // New data available to read from RailHub
    RAILD_EV_UART_TIMER, // UART timer event
    RAILD_EV_SERVER,     // Events from the TCP/IP server
    RAILD_EV_SOCKET,     // Events from a TCP/IP client
    RAILD_EV_LUA_TIMER,  // Timer created by Lua scripts
} raild_event_type;

// user data struct for epoll events
// This struct holds various informations about an event and
// is the main object passed around when dealing with events
// and epoll.
typedef struct raild_event_t {
    int                   fd;    // The associated file descriptor
    raild_event_type      type;  // Event type flag
    struct timespec       time;  // Event trigger timestamp
    bool                  timer; // TRUE if this event is a timer
    int                   times; // Number of times this timer was triggered since the last event
    int                   n;     // User-defined number
    void                 *ptr;   // User-defined pointer
    bool                  purge; // True when this event is ready to be collected
} raild_event;

// A single byte of 8-bit used for communication with RailHub
typedef unsigned char rbyte;

typedef struct {
    const bool sim;
    const int  lua_init;
} options_t;

extern options_t options;

//---------------------------------------------------------------------------//
// SETUP
//---------------------------------------------------------------------------//
void setup_socket();
void setup_uart();
void setup_gpio();
void setup_lua(const char *main);

void parse_options(int argc, char **argv);

//---------------------------------------------------------------------------//
// GPIO
//---------------------------------------------------------------------------//
void set_gpio(bool state);

//---------------------------------------------------------------------------//
// State
//---------------------------------------------------------------------------//
void  set_hub_state(rhub_port port, rbyte value);
rbyte get_hub_state(rhub_port port);
void  set_hub_readiness(bool r);
bool  get_hub_readiness();
void  set_power(bool p);
bool  get_power();

//---------------------------------------------------------------------------//
// Epoll wrappers
//---------------------------------------------------------------------------//
extern struct epoll_event *epoll_events;

void         raild_epoll_create();
raild_event *raild_epoll_add(int fd, raild_event_type type);
void         raild_epoll_rem(raild_event *udata);
void         raild_epoll_purge(raild_event *event);
int          raild_epoll_wait();
raild_event *event_data(int n);

//---------------------------------------------------------------------------//
// Timers
//---------------------------------------------------------------------------//
raild_event *raild_timer_create(int initial, int interval, raild_event_type type);
void         raild_timer_delete(raild_event *event);
void         raild_timer_autodelete(raild_event *event);

//---------------------------------------------------------------------------//
// UART
//---------------------------------------------------------------------------//
void uart_reset();
void uart_setswitch_on(rbyte sid);
void uart_setswitch_off(rbyte sid);
void uart_setpower(bool state);
void uart_handle_event(raild_event *event);
void uart_handle_timer(raild_event *event);

//---------------------------------------------------------------------------//
// Socket
//---------------------------------------------------------------------------//
void socket_handle_server(raild_event *event);
void socket_handle_client(raild_event *event);

//---------------------------------------------------------------------------//
// Lua
//---------------------------------------------------------------------------//
void lualib_register();
void lua_eval(const char *buffer, size_t length);

void lua_alloc_context(int fd, const char *cls);
void lua_dealloc_context(int fd);
void lua_switch_context(int fd);
void lua_restore_context();

void lua_handle_timer(raild_event *event);
void lua_delete_timer(void *timer);

void lua_oninit();
void lua_onready();
void lua_ondisconnect();
void lua_onpower(bool state);
void lua_onsensorchange(int sensorid, bool state);
void lua_onswitchchange(int switchid, bool state);

//---------------------------------------------------------------------------//
// Logger
//---------------------------------------------------------------------------//
void  logger(const char *prefix, const char* message);
void  logger_error(const char* message);
void  logger_light(const char* message);
char* logger_prefix(const char* prefix, const char* message);

#endif
