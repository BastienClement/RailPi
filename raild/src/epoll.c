#include "raild.h"
#include <sys/epoll.h>

/*
 * epoll() wrapper for raild
 *
 * This file contains helpers functions to encapsulate the epoll() API
 * to a simpler API restricted for usages required in raild.
 */

// Maximum time to wait for events before timeout
#define WAIT_TIMEOUT 1000

// Maximum number of events to return in one epoll_wait() call
#define MAX_EVENTS   64

// The epoll file descriptor used to manipulate watched fds
static int efd;

// An epoll_events array used to store results from epoll_wait()
struct epoll_event *epoll_events;

//
// Creates a new epoll instance
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
	// Create the raild_event object associated with this epoll instance
	raild_event *event = malloc(sizeof(raild_event));
	event->fd    = fd;
	event->type  = type;
	event->timer = false;
	event->n     = 0;
	event->ptr   = 0;

	// Setting up everything required by epoll
	struct epoll_event epoll_ev;
	epoll_ev.data.fd  = fd;      // The watched file descriptor
	epoll_ev.data.ptr = event;   // Pointer to the raild_event object
	epoll_ev.events   = EPOLLIN; // Watching for reading-available events

	// Effectively add the fd to epoll
	int s = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &epoll_ev);
	if(s < 0) {
		perror("epoll_ctl");
		exit(1);
	}

	return event;
}

//
// Removes a fd from the event loop
// Must use the corresponding raild_event object for it to be freed.
// (it's why you can't just pass the fd to remove as parameter)
//
void raild_epoll_rem(raild_event *event) {
	epoll_ctl(efd, EPOLL_CTL_DEL, event->fd, 0);
	free(event);
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
