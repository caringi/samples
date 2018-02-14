/*
 * Reverted Pascal Triangle TCP Server
 *
 * Author: Augusto Mecking Caringi (augustocaringi@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT	50000

int exit_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void send_reverted_pascal_triangle(FILE *fd, int n)
{
	int	c, i, j;

	for (i = (n - 1); i >= 0; i--) {
		c = 1;
		for (j = 0; j <= i; j++) {
			fprintf(fd, "%d ", c);
			c = c * (i - j)/(j + 1);
		}
		fprintf(fd, "\n");
	}
}

void connection_handler(int s)
{
	uint32_t	value;
	char		buffer[50], *endptr;
	FILE		*fd;

	fd = fdopen(s, "r+");
	setlinebuf(fd);
	fgets(buffer, sizeof(buffer), fd);
	value = strtoul(buffer, &endptr, 10);
	if (*endptr != 0xd && *endptr != 0xa)
		fprintf(fd, "error: invalid input\n");
	else
		send_reverted_pascal_triangle(fd, value);
	fclose(fd);
}

void server_start(uint16_t port)
{
	struct sockaddr_in	serv_addr, cli_addr;
	socklen_t		clilen;
	int			listenfd, newsockfd;
	pid_t			pid;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit_error("error opening socket...");
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		exit_error("error on binding...");
	listen(listenfd, 5);
	clilen = sizeof(cli_addr);
	printf("server is running on port %d\n", port);
	// Main server loop
	for (;;) {
		if ((newsockfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen)) < 0)
			exit_error("error on accept...");
		if ((pid = fork()) == 0) {
			// Child
			connection_handler(newsockfd);
			exit(EXIT_SUCCESS);
		} else if (pid > 0) {
			// Parent
			close(newsockfd);
		} else {
			fprintf(stderr, "fork() failed...\n");
		}
	}
}

int main(int argc, char *argv[])
{
	uint16_t	port;
	char		*endptr;

	if (argc > 2) {
		exit_error("invalid number or arguments...");
	} else if (argc == 2) {
		port = strtoul(argv[1], &endptr, 10);
		if (*endptr !='\0')
			exit_error("invalid parameter...");
		else if (port < 1 || port > UINT16_MAX)
			exit_error("invalid port number...");
	} else {
		port = DEFAULT_PORT;
	}

	server_start(port);

	return EXIT_SUCCESS;
}
