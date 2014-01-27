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

// fd type indicator for epoll events
typedef enum {
	RAILD_FD_UART,
	RAILD_FD_SERVER,
	RAILD_FD_SOCKET
} epoll_type;

typedef enum {
	RHUB_SENSORS1,
	RHUB_SENSORS2,
	RHUB_SENSORS3,
	RHUB_SWITCHES
} rhub_port;

// udata struct for epoll events
typedef struct epoll_udata_t {
	struct epoll_udata_t *self;
	int                   fd;
	epoll_type            type;
	struct timespec       time;
	bool                  timer;
	int                   times;
	uint32_t              n;
	void                 *ptr;
} epoll_udata;

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
epoll_udata *raild_epoll_add(int fd, epoll_type type);
void         raild_epoll_rem(epoll_udata *udata);
int          raild_epoll_wait();
epoll_udata *event_udata(int n);

//
// --- Timers ---
//
int  raild_timer_create(int initial, int interval, epoll_type type);
void raild_timer_delete(int tid);
void raild_timer_autodelete(epoll_udata *udata);

//
// --- UART ---
//
void uart_reset();
void uart_handle_event(epoll_udata *udata);

//
// --- Lua ---
//
int  lua_onready();
void lualib_register();

#endif
