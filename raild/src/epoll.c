#include "raild.h"
#include <sys/epoll.h>

#define WAIT_TIMEOUT 1000
#define MAX_EVENTS   64

static int efd;
struct epoll_event *epoll_events;

//
// Creates a new epoll instance and registers UART fd and Server fd
//
void raild_epoll_create() {
	efd = epoll_create1(0);
	if(efd < 0) {
		perror("epoll_create");
		exit(1);
	}

	epoll_events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
}

//
// Adds a new fd to the epoll instance
//
epoll_udata *raild_epoll_add(int fd, epoll_type type) {
	epoll_udata *udata = malloc(sizeof(epoll_udata));
	udata->self  = udata;
	udata->fd    = fd;
	udata->type  = type;
	udata->timer = false;
	udata->n     = 0;
	udata->ptr   = 0;

	struct epoll_event event;
	event.data.fd  = fd;
	event.data.ptr = udata;
	event.events   = EPOLLIN;

	int s = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
	if(s < 0) {
		perror("epoll_ctl");
		exit(1);
	}

	return udata;
}

//
// Removes a fd from the event loop
//
void raild_epoll_rem(epoll_udata *udata) {
	epoll_ctl(efd, EPOLL_CTL_DEL, udata->fd, 0);
	free(udata->self);
}

//
// Simple wrapper around epoll wait
//
int raild_epoll_wait() {
	return epoll_wait(efd, epoll_events, MAX_EVENTS, WAIT_TIMEOUT);
}

//
// Returns the udata struct for the nth event
//
epoll_udata *event_udata(int n) {
	return (epoll_udata *) epoll_events[n].data.ptr;
}
