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
			raild_event *event = event_data(i);

			// Add time informations
			event->time = tp;

			// Filter function for timers
			if(event->timer) {
				uint64_t times;
				if(read(event->fd, &times, 8) != 8) {
					continue;
				} else {
					event->times = (int) times;
				}
			}

			// Dispatch events
			switch(event->type) {
				case RAILD_EV_UART:
					uart_handle_event(event);
					break;

				case RAILD_EV_LUA_TIMER:
					lua_handle_timer(event);
					break;

				default:
					printf("[EPOLL]\t Got event on an unmanageable fd type\n");
					exit(1);
			}

			// Auto delete feature
			if(event->timer) {
				raild_timer_autodelete(event);
			}
		}
	}

	return 0;
}
