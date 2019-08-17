#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SYSCALL(estat, func, args...)					\
({									\
	int __ret;							\
									\
									\
	while ((__ret = ((func)(args))) == -1 && errno == EINTR)	\
		;							\
									\
	if (__ret == -1) {						\
		perror(#func "(" #args ")");				\
		exit(estat);						\
	}								\
									\
	__ret;								\
})

#define MSG_SIZE 4000

int
server(void)
{
	int fd;
	int ret;
	char msg[MSG_SIZE];
	int broadcast = 1;
	fd_set fd_set;
	struct timeval timeout;
	socklen_t socklen = sizeof (struct sockaddr_in);
	struct sockaddr_in addr_cli;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(5432),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};


	fd = SYSCALL(1, socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SYSCALL(1, bind, fd, (struct sockaddr*)&addr, socklen);
	SYSCALL(1, setsockopt, fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (int));

	while (1) {
		FD_ZERO(&fd_set);
		FD_SET(fd, &fd_set);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		if (SYSCALL(1, select, fd + 1, &fd_set, NULL, NULL, &timeout) == 0)
			continue;

		ret = SYSCALL(1, recvfrom, fd, msg, MSG_SIZE - 1, 0,
			    (struct sockaddr*)&addr_cli, &socklen);
		msg[ret] = '\0';

		printf("%s", msg);
	}

	return 0;
}

int
client(void)
{
	int fd;
	char msg[] = "Hello World\n";
	int broadcast = 1;
	socklen_t socklen = sizeof (struct sockaddr_in);
	struct sockaddr_in addr_brd = {
		.sin_family = AF_INET,
		.sin_port = htons(5432),
		.sin_addr.s_addr = htonl(INADDR_BROADCAST),
	};
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(0),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};


	fd = SYSCALL(1, socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SYSCALL(1, bind, fd, (struct sockaddr*)&addr, socklen);
	SYSCALL(1, setsockopt, fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (int));

	while (1) {
		SYSCALL(1, sendto, fd, msg, strlen(msg), 0, (struct sockaddr*)&addr_brd,
			socklen);

		sleep(5);
	}

	return 0;
}

int
main(void)
{
	pid_t pid;


	pid = fork();

	if (pid > 0)
		return server();
	else if (pid == 0)
		return client();
	else {
		perror("fork()");
		return 1;
	}
}
