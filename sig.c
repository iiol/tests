#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

pthread_t tid;
int th_exist;


void*
thread(void *args)
{
        while (1) {
                printf("Hi thread\n");
                sleep(1);
        }

        return NULL;
}

void
usr1(int func)
{
        pthread_attr_t attr;


        if (th_exist)
                return;

        if (pthread_attr_init(&attr)) {
                fprintf(stderr, "pthread_attr_init(&attr)");
                return;
        }

        if ((errno = pthread_create(&tid, &attr, thread, NULL)) != 0) {
                perror("pthread_create()");
                return;
        }

        th_exist = 1;

        return;
}

void
usr2(int func)
{
        if (!th_exist)
                return;

        pthread_cancel(tid);
        pthread_join(tid, NULL);

        th_exist = 0;

        return;
}

int
main(void)
{
        int fd;


        fd = open("pid", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dprintf(fd, "%d\n", getpid());

        if (signal(SIGUSR1, usr1) == SIG_ERR) {
                perror("signal(SIGUSR1, usr1)");
                return 1;
        }

        if (signal(SIGUSR2, usr2) == SIG_ERR) {
                perror("signal(SIGUSR2, usr2)");
                return 1;
        }

        while (1)
                sleep(20);

        return 0;
}
