#include "raild.h"
#include <sys/timerfd.h>

static struct timespec _split_time(int time) {
	struct timespec ts;
	ts.tv_sec  = time / 1000;
	ts.tv_nsec = (time % 1000) * 1000000;
	return ts;
}

int raild_timer_create(int initial, int interval, epoll_type type) {
	int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);

	struct itimerspec timer;
	timer.it_value    = _split_time(initial);
	timer.it_interval = _split_time(interval);

	timerfd_settime(fd, 0, &timer, NULL);
	raild_epoll_add(fd, type)->timer = true;

	return fd;
}

void raild_timer_delete(int fd) {
	close(fd);
}

void raild_timer_autodelete(epoll_udata *udata) {
	struct itimerspec timer;
	if(timerfd_gettime(udata->fd, &timer) < 0) {
		perror("timerfd_gettime");
		exit(1);
	}

	if(timer.it_interval.tv_sec == 0 && timer.it_interval.tv_nsec == 0) {
		raild_epoll_rem(udata);
		raild_timer_delete(udata->fd);
	}
}
