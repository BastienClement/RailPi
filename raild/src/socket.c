#include "raild.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

static int sockfd;

#define BUFFER_MAX_LEN 4096

typedef struct {
	char *buffer;
	int   buffer_len;
} client_data;

int setup_socket() {
	printf("[API]\t Init API server\n");

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0) {
		perror("socket");
		exit(1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(9000);

	int optval = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	if(ioctl(sockfd, FIONBIO, &optval) < 0) {
		perror("ioctl");
		exit(1);
	}

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	if(listen(sockfd, 5) < 0) {
		perror("listen");
		exit(1);
	}

	raild_epoll_add(sockfd, RAILD_EV_SERVER);
	return sockfd;
}

void socket_handle_server(raild_event *event) {
	printf("[API]\t New client connected\n");

	client_data *cdata = malloc(sizeof(client_data));
	cdata->buffer      = malloc(sizeof(char[BUFFER_MAX_LEN]));
	cdata->buffer_len  = 0;

	int clientfd = accept(sockfd, NULL, NULL);
	raild_epoll_add(clientfd, RAILD_EV_SOCKET)->ptr = cdata;
}

void _close(raild_event *event) {
	close(event->fd);
	free(((client_data *) event->ptr)->buffer);
	free(event->ptr);
	raild_epoll_rem(event);
}

void socket_handle_client(raild_event *event) {
	client_data *cdata = (client_data *) event->ptr;

	// Read data in buffer
	int len = read(event->fd, (cdata->buffer + cdata->buffer_len), (BUFFER_MAX_LEN - cdata->buffer_len));
	cdata->buffer_len += len;

	// Check some space exists in the client buffer
	if(cdata->buffer_len == BUFFER_MAX_LEN) {
		printf("[API]\t Client buffer overflow, kicking\n");
		_close(event);
		return;
	}

	// Connection closed
	if(len == 0) {
		printf("[API]\t Client disconnected\n");
		_close(event);
		return;
	}

	// Scan buffer for code block
	char *buffer = cdata->buffer;
	int   length = cdata->buffer_len;
	for(int i = 0; i < length; i++) {
		if(buffer[i] == '\f') {
			if(i > 0) {
				lua_set_context(event->fd);
				lua_eval(buffer, i);
				lua_clear_context();
			}

			buffer += i + 1;
			length -= i + 1;
			i = 0;
		}
	}

	// Move end of buffer to the beginning
	if(cdata->buffer_len != length) {
		memmove(cdata->buffer, buffer, length);
		cdata->buffer_len = length;
	}
}
