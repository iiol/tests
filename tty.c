#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>

int printflag;
int fd;

void*
function(void *args)
{
        int i = 0;

        for (i = 0;;) {
                if (printflag) {
                        printf("%d) Hello World\n", i++);
                        sleep(1);
                }
        }
}

void
usr1(int sig)
{
        printf("Usr1\n");
        printflag = 1;
        ioctl(fd, VT_RELDISP, VT_ACKACQ);
}

void
usr2(int sig)
{
        printf("Usr2\n");
        printflag = 0;
	ioctl(fd, VT_RELDISP, 1);
}

int
main(void)
{
        struct stat stat;
        struct vt_stat vts;
        struct vt_mode mode;

        pthread_t tid;
        pthread_attr_t attr;

// set signal handlers
        if (signal(SIGUSR1, usr1) == SIG_ERR) {
                perror("signal(SIGUSR1, usr1)");
                return 1;
        }

        if (signal(SIGUSR2, usr2) == SIG_ERR) {
                perror("signal(SIGUSR2, usr2)");
                return 1;
        }


// open tty and save stat
        fd = open("/dev/tty7", O_RDWR | O_NOCTTY | O_CLOEXEC);
        if (fd < 0) {
                perror("open()");
                return 1;
        }

        if (fstat(fd, &stat)) {
                perror("fstat()");
                return 1;
        }


// save vt stat
        if (ioctl(fd, VT_GETSTATE, &vts)) {
                perror("ioctl(fd, VT_GETSTATE, &vts)");
                return 1;
        }

// set kd mode
        if (ioctl(fd, KDSETMODE, KD_GRAPHICS)) {
                perror("ioctl(fd, KDSETMODE, KD_GRAPHICS)");
                return 1;
        }

// get current vt mode, change and set them
        if (ioctl(fd, VT_GETMODE, &mode)) {
                perror("ioctl(fd, VT_GETMODE, &mode)");
                return 1;
        }

        mode.mode = VT_PROCESS;
        mode.acqsig = SIGUSR1;
        mode.relsig = SIGUSR2;

        if (ioctl(fd, VT_SETMODE, &mode)) {
                perror("ioctl(fd, VT_SETMODE, &mode)");
                return 1;
        }

        printf("pid: %d\n", getpid());
        printf("VT_ACKACQ: %d\n", VT_ACKACQ);


// pthread create
        if (pthread_attr_init(&attr)) {
                fprintf(stderr, "pthread_arrt_init(&attr)\n");
                return 1;
        }

        if (errno = pthread_create(&tid, &attr, function, NULL)) {
                perror("pthread_create()");
                return 1;
        }

// wait sig{1,2} or thread exiting
        pthread_join(tid, NULL);

        return 0;
}
