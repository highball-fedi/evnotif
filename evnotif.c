#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char** argv){
	const char* host = NULL;
	const char* port = "6667";

	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;

	int s;
	int sfd;
	int i;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "--host") == 0){
			host = argv[++i];
		}else if(strcmp(argv[i], "--port") == 0){
			port = argv[++i];
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
	if(s != 0) return s;

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
	return 0;
}
