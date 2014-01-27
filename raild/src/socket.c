#include "raild.h"
#include <sys/socket.h>
#include <netinet/in.h>

int setup_socket() {
	printf("[API]\t Init API server\n");

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0) {
		perror("socket");
		exit(1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(9000);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	if(listen(sockfd, 5) < 0) {
		perror("listen");
		exit(1);
	}

	raild_epoll_add(sockfd, RAILD_FD_SERVER);
	return sockfd;
}
