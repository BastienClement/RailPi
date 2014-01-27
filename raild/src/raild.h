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
#include <string.h>
#include <errno.h>
#include <sys/types.h>

// debug
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
typedef struct {
	epoll_type  type;
	void       *udata;
} epoll_udata;

typedef unsigned char rbyte;

//
// --- SETUP ---
//
int setup_socket();
int setup_uart();
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
void raild_epoll_create();
void raild_epoll_add(int fd, epoll_type type, void *udata);
int  raild_epoll_wait();
epoll_udata *event_udata(int n);
int event_fd(int n);

//
// --- UART ---
//
void uart_handle_event(int fd, void *udata);

//
// --- Lua ---
//
int lua_onready();
void lualib_register();

#endif
