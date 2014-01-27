#include "raild.h"

int main(int argc, char **argv) {
	printf("[RAILD]\t Starting raild...\n");

	// --- SETUP ---

	// Epoll
	raild_epoll_create();

	// UART
	setup_uart();

	// Lua
	setup_lua((argc > 1) ? argv[1] : NULL);

	// Socket
	setup_socket();

	printf("[RAILD]\t Setup completed!\n");
	lua_onready();

	while(1) {
		int n = raild_epoll_wait();
		if(n < 1) {
			// Event loop idle
			continue;
		}

		struct timespec tp;
		clock_gettime(CLOCK_MONOTONIC, &tp);

		for(int i = 0; i < n; i++) {
			// Extract fd and udata from the event
			epoll_udata *udata = event_udata(i);

			// Add time informations
			udata->time = tp;

			// Filter function for timers
			if(udata->timer) {
				uint64_t times;
				if(read(udata->fd, &times, 8) != 8) {
					continue;
				} else {
					udata->times = (int) times;
				}
			}

			// Dispatch events
			switch(udata->type) {
				case RAILD_FD_UART:
					uart_handle_event(udata);
					break;

				default:
					printf("[EPOLL]\t Got event on an unmanageable fd type\n");
					exit(1);
			}

			// Auto delete feature
			if(udata->timer) {
				raild_timer_autodelete(udata);
			}
		}
	}

	return 0;
}
