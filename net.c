#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>


#define MSG_SIZE 60000
#define SOCKLEN sizeof (struct sockaddr_in)
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


int
net_init(char *address, int port)
{
	int fd;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr = inet_addr(address),
		.sin_port = htons(port),
	};


	fd = SYSCALL(1, socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SYSCALL(1, bind, fd, (struct sockaddr*)&addr, SOCKLEN);

	return fd;
}

int
server1(void)
{
	int fd, n;
	char msg[MSG_SIZE];
	socklen_t socklen = SOCKLEN;
	struct sockaddr_in addr;


	fd = net_init("0.0.0.0", 55555);

	while (1) {
		n = SYSCALL(1, recvfrom, fd, msg, MSG_SIZE - 1, 0,
			    (struct sockaddr*)&addr, &socklen);
		msg[n] = '\0';

		printf("SERVER1: msg:\n----------\n%s\n----------\n", msg);
	}

	return 1;
}

int
server2(void)
{
	return 0;
	int fd, n;
	char msg[MSG_SIZE];
	socklen_t socklen = SOCKLEN;
	struct sockaddr_in addr;


	fd = net_init("0.0.0.0", 55555);

	while (1) {
		n = SYSCALL(1, recvfrom, fd, msg, MSG_SIZE - 1, 0,
			    (struct sockaddr*)&addr, &socklen);
		msg[n] = '\0';

		printf("SERVER2: msg:\n----------\n%s\n----------\n", msg);
	}

	return 1;
}

int
client1(void)
{
	int fd;
	int broadcast = 1;
	char msg[] = "Hi World";
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		//.sin_addr.s_addr = htonl(INADDR_BROADCAST),
		.sin_addr = inet_addr("127.255.255.255"),
		.sin_port = htons(55555),
	};


	fd = net_init("0.0.0.0", 0);
	SYSCALL(1, setsockopt, fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (int));

	while (1) {
		SYSCALL(1, sendto, fd, msg, strlen(msg), 0, (struct sockaddr*)&addr,
			SOCKLEN);

		sleep(5);
	}
}

int
client2(void)
{
}

int
main(void)
{
	pid_t pid;


	pid = SYSCALL(1, fork);
	if (pid == 0)
		return server1();

	pid = SYSCALL(1, fork);
	if (pid == 0)
		return server2();

	pid = SYSCALL(1, fork);
	if (pid == 0)
		return client1();

	pid = SYSCALL(1, fork);
	if (pid == 0)
		return client2();

	wait(NULL);
	wait(NULL);
	wait(NULL);

	return 0;
}
