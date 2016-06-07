#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include <errno.h>

#define BUFSIZE 2000

int
main(int argc, char *argv[])
{
	int sockfd;			/* socket */
	int portno;			/* port to listen on */
	unsigned int clientlen;		/* byte size of client's address */
	struct sockaddr_in clientaddr;	/* client addr */
	struct sockaddr_in serveraddr;
	unsigned int to_len;
	struct sockaddr_in to;

	char buf[BUFSIZE];	/* message buf */
	int optval;		/* flag value for setsockopt */
	int n;			/* message byte size */
	int portshift;		/* shift destination port depending on source address */
	int cutbytes = 0;

	if (argc > 1) {
		portno = atoi(argv[1]);
	} else {
		portno = 60001;
	}

	if (argc > 2) {
		inet_pton(AF_INET, argv[2], &(to.sin_addr));
	} else {
		inet_pton(AF_INET, "192.168.0.0", &(to.sin_addr));
	}
	portshift = ntohl(to.sin_addr.s_addr);

	if (argc > 3) {
		cutbytes = atoi(argv[3]);
	}

	if (!inet_pton(AF_INET, "127.0.0.1", &(to.sin_addr))) {
		fprintf(stderr, "inet_pton() failed\n");
		return EXIT_FAILURE;
	}
	to.sin_family = AF_INET;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Can't open socket");
		return EXIT_FAILURE;
	}

	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
#ifdef SO_REUSEPORT
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
#endif

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		fprintf(stderr, "Can't bind socket: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	clientlen = sizeof(clientaddr);
	to_len = sizeof(to);
	for (;;) {
		int port;

		/* receive a UDP datagram */
		n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
		if (n < 0) {
			fprintf(stderr, "Error in recvfrom\n");
			continue; /* FIXME: exit? */
		}

		if (n <= cutbytes) {
			fprintf(stderr, "Short datagram, %d bytes only\n", n);
			continue;
		}

		port = ntohs(clientaddr.sin_port);
		port += (ntohl(clientaddr.sin_addr.s_addr) - portshift) * 10;

		/* debug output */
#if 0
		fprintf(stderr, "p: %d, hdr: ", port);
		for (i=0; i<n; i++) {
			fprintf(stderr, "%3x|", (unsigned char)buf[i]);
		}
		fprintf(stderr, "\n");


		if (!inet_ntop(AF_INET, &(clientaddr.sin_addr), hostaddrp, INET_ADDRSTRLEN)) {
			fprintf(stderr, "Error in inet_ntop\n");
			continue; /* FIXME: exit? */
		}
		printf("server received datagram from %s:%d\n", hostaddrp, port);
		printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
#endif
    
		/* send datagram */
		to.sin_port = htons(port);
		n = sendto(sockfd, buf + cutbytes, n - cutbytes, 0, (struct sockaddr *) &to, to_len);
		if (n < 0) {
			fprintf(stderr, "Error in sendto: %s\n", strerror(errno));
			continue; /* FIXME: exit? */
		}
	}

	return EXIT_SUCCESS;
}

