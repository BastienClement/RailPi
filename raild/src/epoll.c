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
raild_event *raild_epoll_add(int fd, raild_event_type type) {
	raild_event *event = malloc(sizeof(raild_event));
	event->self  = event;
	event->fd    = fd;
	event->type  = type;
	event->timer = false;
	event->n     = 0;
	event->ptr   = 0;

	struct epoll_event epoll_ev;
	epoll_ev.data.fd  = fd;
	epoll_ev.data.ptr = event;
	epoll_ev.events   = EPOLLIN;

	int s = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &epoll_ev);
	if(s < 0) {
		perror("epoll_ctl");
		exit(1);
	}

	return event;
}

//
// Removes a fd from the event loop
//
void raild_epoll_rem(raild_event *event) {
	epoll_ctl(efd, EPOLL_CTL_DEL, event->fd, 0);
	free(event->self);
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
raild_event *event_data(int n) {
	return (raild_event *) epoll_events[n].data.ptr;
}
