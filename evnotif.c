#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO 0
#define IRC 1

const char* host = NULL;
const char* port = "6667";
const char* fifo = "/tmp/evnotif";
const char* user = "EvNotif";
const char* channel = NULL;

int sfd;
int ffd;

int do_fifo(void){
	return 0;
}

int irc_sp = 0;
int irc_host = -1;
char irc_job = 0;
char* irc_buffer;
int do_irc(void){
	char c;
	if(recv(sfd, &c, 1, 0) <= 0) return 1;
	if(c == ' ' && irc_sp == 0){
		irc_sp++;
	}else if(c == ' ' && irc_sp == 1){
		if(strcasecmp(irc_buffer, "PING") == 0){
			irc_job = 'P';
		}
		free(irc_buffer);
		irc_buffer = malloc(1);
		irc_buffer[0] = 0;
		irc_sp++;
	}else if(c == '\r'){
	}else if(c == '\n'){
		irc_sp = 0;
		irc_host = -1;
		if(irc_job == 'P'){
			send(sfd, "PONG ", 5, 0);
			send(sfd, ":not.configured", strlen(":not.configured"), 0);
			send(sfd, "\r\n", 2, 0);
		}
		printf("[%s]\n", irc_buffer);
		irc_job = 0;
		free(irc_buffer);
		irc_buffer = malloc(1);
		irc_buffer[0] = 0;
	}else{
		if(irc_host == -1){
			irc_host = c == ':' ? 1 : 0;
			if(!irc_host) irc_sp++;
		}
		if(irc_sp > 0){
			int len = strlen(irc_buffer);
			char* old = irc_buffer;
			irc_buffer = malloc(len + 2);
			strcpy(irc_buffer, old);
			free(old);
			irc_buffer[len] = c;
			irc_buffer[len + 1] = 0;
		}
	}
	return 0;
}

int main(int argc, char** argv){
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;

	int s;
	int i;

	struct pollfd* pfds = malloc(sizeof(*pfds) * 2);

	irc_buffer = malloc(1);
	irc_buffer[0] = 0;

	for(i = 0; i < 2; i++) pfds[i].events = POLLIN | POLLPRI;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "--host") == 0){
			host = argv[++i];
		}else if(strcmp(argv[i], "--port") == 0){
			port = argv[++i];
		}else if(strcmp(argv[i], "--fifo") == 0){
			fifo = argv[++i];
		}else if(strcmp(argv[i], "--user") == 0){
			user = argv[++i];
		}else if(strcmp(argv[i], "--channel") == 0){
			channel = argv[++i];
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
	if(user == NULL){
		fprintf(stderr, "User is empty\n");
		s = 1;
	}
	if(channel == NULL){
		fprintf(stderr, "Channel is empty\n");
		s = 1;
	}
	if(s != 0) return s;

	remove(fifo);
	mkfifo(fifo, 0666);
	ffd = open(fifo, O_RDWR);
	if(ffd == -1){
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
		int yes = 1;
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
		if(sfd == -1) continue;
		if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) break;
		close(sfd);
	}
	freeaddrinfo(result);
	if(rp == NULL){
		fprintf(stderr, "Could not connect\n");
		return 1;
	}
	pfds[FIFO].fd = ffd;
	pfds[IRC].fd = sfd;
	printf("Connected\n");
	send(sfd, "USER ", 5, 0);
	send(sfd, user, strlen(user), 0);
	send(sfd, " ", 1, 0);
	send(sfd, user, strlen(user), 0);
	send(sfd, " ", 1, 0);
	send(sfd, user, strlen(user), 0);
	send(sfd, " ", 1, 0);
	send(sfd, user, strlen(user), 0);
	send(sfd, "\r\n", 2, 0);
	send(sfd, "NICK ", 5, 0);
	send(sfd, user, strlen(user), 0);
	send(sfd, "\r\n", 2, 0);
	send(sfd, "JOIN ", 5, 0);
	send(sfd, channel, strlen(channel), 0);
	send(sfd, "\r\n", 2, 0);
	while(1){
		s = poll(pfds, 2, 1000);
		if(s < 0) break;
		if(s > 0){
			s = 0;
			for(i = 0; i < 2; i++){
				if(pfds[i].revents & POLLIN){
					if(i == FIFO) s += do_fifo();
					if(i == IRC) s += do_irc();
				}
			}
			if(s > 0) break;
		}
	}
	return 0;
}
