#include "raild.h"

int tick_interval = 1000;

int main(int argc, char **argv) {
	printf("[RAILD]\t Starting raild...\n");

	// --- SETUP ---

	// UART
	int uart_fd = setup_uart();

	// Lua
	setup_lua((argc > 1) ? argv[1] : NULL);

	// Socket
	int serv_fd = setup_socket();

	// Epoll
	raild_epoll_create(uart_fd, serv_fd);

	printf("[RAILD]\t Setup completed!\n");
	lua_onready();

	/*struct sockaddr_in cli_addr;
	size_t cli_addr_len = sizeof(cli_addr);*/

	while(1) {
		int n = raild_epoll_wait();
		if(n < 1) {
			printf("Timeout exception\n");
			//break;
		}

		for(int i = 0; i < n; i++) {
			epoll_udata *udata = event_udata(i);
			switch(udata->type) {
				case RAILD_FD_UART:
					uart_handle_event(event_fd(i));
					break;

				default:
					printf("[EPOLL]\t Got event on an unmanageable fd type\n");
					exit(1);
			}
		}

		/*int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addr_len);
		FILE* fd = fdopen(newsockfd, "w");
		if(newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}
		fputs("Hello world!\n", fd);
		fclose(fd);
		close(newsockfd);*/
	}

	return 0;
}
