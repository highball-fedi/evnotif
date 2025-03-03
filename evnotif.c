#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO 0
#define IRC 1

const char* host = NULL;
const char* port = "6667";
const char* fifo = "/tmp/evnotif";

int main(int argc, char** argv){
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;

	int s;
	int sfd;
	int i;

	struct pollfd* pfds = malloc(sizeof(*pfds) * 2);

	for(i = 0; i < 2; i++) pfds[i].events = POLLIN | POLLPRI;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "--host") == 0){
			host = argv[++i];
		}else if(strcmp(argv[i], "--port") == 0){
			port = argv[++i];
		}else if(strcmp(argv[i], "--fifo") == 0){
			fifo = argv[++i];
		}else{
			fprintf(stderr, "%s: invalid argument\n", argv[i]);
			return 1;
		}
	}

	s = 0;
	if(host == NULL){
		fprintf(stderr, "Host is empty\n");
		s = 1;
	}
	if(port == NULL){
		fprintf(stderr, "Port is empty\n");
		s = 1;
	}
	if(fifo == NULL){
		fprintf(stderr, "FIFO is empty\n");
		s = 1;
	}
	if(s != 0) return s;

	remove(fifo);
	mkfifo(fifo, 0666);
	pfds[FIFO].fd = open(fifo, O_RDWR);
	if(pfds[FIFO].fd == -1){
		fprintf(stderr, "open: %s\n", strerror(errno));
		return 1;
	}

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	s = getaddrinfo(host, port, &hints, &result);
	if(s != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return 1;
	}
	for(rp = result; rp != NULL; rp = rp->ai_next){
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd == -1) continue;
		if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) break;
		close(sfd);
	}
	freeaddrinfo(result);
	if(rp == NULL){
		fprintf(stderr, "Could not connect\n");
		return 1;
	}
	pfds[IRC].fd = sfd;
	printf("Connected\n");
	while(1){
		s = poll(pfds, 2, 1000);
		if(s < 0) break;
		if(s > 0){
			for(i = 0; i < 2; i++){
				if(pfds[i].revents & POLLIN){
				}
			}
		}
	}
	return 0;
}
