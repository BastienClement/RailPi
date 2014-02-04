#include "raild.h"

/*
 * Raild event loop
 *
 * The event loop is responsible for waiting, gathering and dispatching
 * events to other modules of raild.
 *
 * Because everything is built around epoll() feature available in Linux
 * kernels, every manageable events have to expose a file description
 * interface as to be compatible with epoll.
 */

int main(int argc, char **argv) {
	printf("[RAILD]\t Starting raild...\n");

	// --- SETUP ---

	// Epoll
	raild_epoll_create();

	// Lua
	setup_lua((argc > 1) ? argv[1] : NULL);

	// UART
	setup_uart();

	// Socket
	setup_socket();

	// GPIO
	setup_gpio();

	printf("[RAILD]\t Setup completed!\n");

	// Main event loop
	// Spinning during the entire life of raild
	while(1) {
		// Wait for events to handle
		int n = raild_epoll_wait();
		if(n < 1) {
			// Event loop idle
			// Just wait again
			continue;
		}

		// Fetching the current time
		struct timespec tp;
		clock_gettime(CLOCK_REALTIME, &tp);

		// Handle each event one by one
		for(int i = 0; i < n; i++) {
			// Extract the raild_event struct from the event
			raild_event *event = event_data(i);

			// Add time informations
			event->time = tp;

			// Filter function for timers
			// Timers fds are automatically read and their 'proc'
			// count is stored in the 'times' field of the raild_event
			if(event->timer) {
				uint64_t times;
				if(read(event->fd, &times, 8) != 8) {
					// This event did not really happened. Just ignore it.
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

				case RAILD_EV_UART_TIMER:
					uart_handle_timer(event);
					break;

				case RAILD_EV_LUA_TIMER:
					lua_handle_timer(event);
					break;

				case RAILD_EV_SERVER:
					socket_handle_server(event);
					break;

				case RAILD_EV_SOCKET:
					socket_handle_client(event);
					break;

				default:
					printf("[EPOLL]\t Got event on an unmanageable fd type\n");
					exit(1);
			}

			// Auto delete feature for non-repeatable timer events
			if(!event->purge && event->timer) {
				raild_timer_autodelete(event);
			}

			// If this event is marked for purge at the end of the event loop
			if(event->purge) {
				raild_epoll_purge(event);
			}
		}
	}

	return 0;
}
