#ifndef RAILD_H
#define RAILD_H 1

//
// --- INCLUDES ---
//
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

//
// --- Types ---
//

typedef enum {
	RHUB_SENSORS1,
	RHUB_SENSORS2,
	RHUB_SENSORS3,
	RHUB_SWITCHES
} rhub_port;

// fd type indicator for epoll events
typedef enum {
	RAILD_EV_UART,
	RAILD_EV_SERVER,
	RAILD_EV_SOCKET,
	RAILD_EV_LUA_TIMER,
} raild_event_type;

// udata struct for epoll events
typedef struct raild_event_t {
	struct raild_event_t *self;
	int                   fd;
	raild_event_type      type;
	struct timespec       time;
	bool                  timer;
	int                   times;
	uint32_t              n;
	void                 *ptr;
} raild_event;

typedef unsigned char rbyte;

//
// --- SETUP ---
//
int  setup_socket();
int  setup_uart();
void setup_lua(const char *main);

//
// --- State ---
//
void set_hub_state(rhub_port port, rbyte value);
void set_hub_readiness(bool r);
bool get_hub_readiness();

//
// --- Main ---
//
extern int tick_interval;

//
// --- Epoll wrappers ---
//
extern struct epoll_event *epoll_events;

void         raild_epoll_create();
raild_event *raild_epoll_add(int fd, raild_event_type type);
void         raild_epoll_rem(raild_event *udata);
int          raild_epoll_wait();
raild_event *event_data(int n);

//
// --- Timers ---
//
raild_event *raild_timer_create(int initial, int interval, raild_event_type type);
void         raild_timer_delete(raild_event *event);
void         raild_timer_autodelete(raild_event *event);

//
// --- UART ---
//
void uart_reset();
void uart_handle_event(raild_event *udata);

//
// --- Lua ---
//
int  lua_onready();
void lua_handle_timer(raild_event *event);
void lualib_register();

#endif
