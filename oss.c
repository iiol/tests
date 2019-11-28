#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SYSCALL(estat, func, args...)					\
({									\
	int __ret;							\
									\
									\
	while ((__ret = ((func)(args))) == -1 && errno == EINTR)		\
		;							\
									\
	if (__ret == -1) {						\
		perror(#func "(" #args ")");				\
		exit(estat);						\
	}								\
									\
	__ret;								\
})

#define AFMT AFMT_S16_NE
#define CHNLS 1
#define RATE 48000

int
main(void)
{
	int i;
	int fd;
	int afmt, chnls, rate;
	char *devname = "/dev/dsp";

	fd = open(devname, O_RDWR, 0);
	if (fd == -1) {
		perror("Dev open error");
		return 1;
	}

	afmt = AFMT;
	chnls = CHNLS;
	rate = RATE;

	rate = SYSCALL(1, ioctl, fd, SNDCTL_DSP_SPEED, &rate);
	chnls = SYSCALL(1, ioctl, fd, SNDCTL_DSP_CHANNELS, &chnls);
	afmt = SYSCALL(1, ioctl, fd, SNDCTL_DSP_SETFMT, &afmt);

	if (rate != 0 && rate < RATE) {
		fprintf(stderr, "Device doesn't support rate.\n");
		return 1;
	}
	if (chnls != 0 && chnls < CHNLS) {
		fprintf(stderr, "Device doesn't support %d channel(s).\n", chnls);
		return 1;
	}
	if (afmt != 0 && afmt < AFMT) {
		fprintf(stderr, "Device doesn't support format.\n");
		return 1;
	}

	while (1) {
		int16_t buf[1024];
		float fbuf[1024];
		size_t size;

		size = 1024 * sizeof (int16_t);

		read(fd, buf, size);

		for (i = 0; i < 1024; ++i)
			fbuf[i] = (float)buf[i]/INT16_MAX;

		for (i = 0; i < 1024; ++i)
			buf[i] = fbuf[i] * INT16_MAX;

		write(fd, buf, size);
	}

	return 0;
}
