#include "raild.h"
#include <sys/timerfd.h>

/**
 * Timer utilities
 *
 * These functions allow simple creation of JavaScript-like timers with
 * milliseconds precision. Those are triggered by event on file descriptor
 * making them quite suitable to use in the event loop.
 */

/**
 * Split time from milliseconds to struct of seconds / nanosecs
 */
static struct timespec _split_time(int time) {
    struct timespec ts;
    ts.tv_sec  = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;
    return ts;
}

/**
 * Create a new timer ticking after `initial` milliseconds then every
 * `interval` milliseconds.
 * If interval is 0, the timer is automatically deleted after firing.
 */
raild_event *raild_timer_create(int initial, int interval, raild_event_type type) {
    // Create the fd timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);

    struct itimerspec timer;
    timer.it_value    = _split_time(initial);
    timer.it_interval = _split_time(interval);

    timerfd_settime(fd, 0, &timer, NULL);

    // Register timer in epoll
    raild_event *event = raild_epoll_add(fd, type);
    event->timer = true;

    return event;
}

/**
 * Delete the timer associated with the event
 */
void raild_timer_delete(raild_event *event) {
    close(event->fd);
    raild_epoll_rem(event);
}

/**
 * Check if the timer need to be automatically deleted and delete it
 */
void raild_timer_autodelete(raild_event *event) {
    struct itimerspec timer;
    if(timerfd_gettime(event->fd, &timer) < 0) {
        perror("timerfd_gettime");
        exit(1);
    }

    if(timer.it_interval.tv_sec == 0 && timer.it_interval.tv_nsec == 0) {
        lua_delete_timer((void *) event);
        raild_timer_delete(event);
    }
}
