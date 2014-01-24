#ifndef RAILD_H
#define RAILD_H 1

//
// --- INCLUDES ---
//
// Common libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/epoll.h>

// Socket server
#include <sys/socket.h>
#include <netinet/in.h>

// UART interface
#include <fcntl.h>
#include <termios.h>
#include <hub_opcodes.h>

// Lua
#include <luajit-2.0/lua.h>
#include <luajit-2.0/lualib.h>
#include <luajit-2.0/lauxlib.h>

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
void raild_epoll_create(int uart_fd, int serv_fd);
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
extern luaL_Reg raild_api[];
int lua_onready();

#endif
