#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

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

#define SWAP(a, b) do {typeof (a) __tmp = a; a = b; b = __tmp;} while (0)
#define sqr(x) ((double)(x)*(x))
#define error(...) fprintf(stderr, __VA_ARGS__)

#define AFMT AFMT_S16_NE
#define CHNLS 1
#define RATE 48000
#define SAMPLES 2048*2

static int
fft(double *rex, double *imx, int n)
{
	int i, ip, j, k, m;
	int le;
	double si, sr, ti, tr, ui, ur;

	if (n <= 0 || rex == NULL || imx == NULL)
		return -1;

	for (i = 1, j = n/2; i < n - 1; ++i, j += k) {
		if (i < j) {
			SWAP(rex[i], rex[j]);
			SWAP(imx[i], imx[j]);
		}

		for (k = n/2; k <= j; j -= k, k /= 2)
			;
	}

	m = log2(n);

	for (k = 1; k <= m; k++) {					//Each stage
		le = pow(2, k - 1);
		sr = cos(M_PI / le);
		si = -sin(M_PI / le);
		ur = 1;
		ui = 0;

		for (j = 1; j <= le; j++) {				//Each SUB DFT
			for (i = j - 1; i < n; i += 2*le) {		//Each butterfly
				ip = i + le;				//Butterfly
				tr = rex[ip] * ur - imx[ip] * ui;
				ti = rex[ip] * ui + imx[ip] * ur;

				rex[ip] = rex[i] - tr;
				imx[ip] = imx[i] - ti;

				rex[i] = rex[i] + tr;
				imx[i] = imx[i] + ti;
			}

			tr = ur;
			ur  = tr * sr - ui * si;
			ui = tr * si + ui * sr;
		}
	}

	return 0;
}

static int
fft_rev(double *rex, double *imx, int n)
{
	int i;

	for (i = 0; i < n; ++i)
		imx[i] = -imx[i];

	fft(rex, imx, n);

	return 0;
}

void
cart_to_polar(double *re, double *im, double *mag, double *phase, int n)
{
	int i;

	for (i = 0; i < n; ++i) {
		mag[i] = sqrt(sqr(re[i]) + sqr(im[i]));
		phase[i] = atan2(im[i], re[i]);
	}
}

void
polar_to_cart(double *mag, double *phase, double *re, double *im, int n)
{
	int i;

	for (i = 0; i < n; ++i) {
		re[i] = mag[i] * cos(phase[i]);
		im[i] = mag[i] * sin(phase[i]);
	}
}

void
filter(double *re, double *im, int n)
{
	int i, j;
	double buf[n];

	memcpy(buf, re, n * sizeof (double));

	for (i = 0; i < n; ++i) {
#if 0
		if (re[i] < 0.00001) {
			re[i] = 0;
			continue;
		}
#endif

		for (j = -100; j < 100; ++j) {
			if (j == 0)
				continue;
			else if (i + j < 0 || i + j >= n)
				continue;

			re[i] += buf[i + j];
			re[i] /= 2;
		}
	}
}

void
shift(double *mag, double *phase, int step, int n)
{
	int i;
	double *buf;

	buf = alloca(n * sizeof (double));

	for (i = 0; i < n; ++i) {
		if (i + step >= n)
			buf[i] = mag[i - n + step];
		else if (i + step < 0)
			buf[i] = mag[i + n + step];
		else
			buf[i] = mag[i + step];
	}

	memcpy(mag, buf, n * sizeof (double));

	for (i = 0; i < n; ++i) {
		if (i + step >= n)
			buf[i] = phase[i - n + step];
		else if (i + step < 0)
			buf[i] = phase[i + n + step];
		else
			buf[i] = phase[i + step];
	}

	memcpy(phase, buf, n * sizeof (double));
}

void
robo(double *mag, double *phase, int n)
{
	memset(phase, 1, n * sizeof (double));
}

int
main(void)
{
	int i;
	int fd;
	int afmt  = AFMT,
	    chnls = CHNLS,
	    rate  = RATE;
	char *devname = "/dev/dsp";
	int16_t buf[SAMPLES];
	double re[SAMPLES], im[SAMPLES];
	double mag[SAMPLES], phase[SAMPLES];

	fd    = SYSCALL(1, open, devname, O_RDWR, 0);
	rate  = SYSCALL(1, ioctl, fd, SNDCTL_DSP_SPEED,    &rate);
	chnls = SYSCALL(1, ioctl, fd, SNDCTL_DSP_CHANNELS, &chnls);
	afmt  = SYSCALL(1, ioctl, fd, SNDCTL_DSP_SETFMT,   &afmt);

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
		read(fd, buf, SAMPLES * sizeof (int16_t));

		for (i = 0; i < SAMPLES; ++i) {
			re[i] = (double)buf[i]/INT16_MAX;
			if (re[i] < 0.001)
				re[i] /= 2;
			im[i] = .0;
		}

		fft(re, im, SAMPLES);

		cart_to_polar(re, im, mag, phase, SAMPLES);
		shift(mag, phase, 0, SAMPLES);
		//robo(mag, phase, SAMPLES/2);
		polar_to_cart(mag, phase, re, im, SAMPLES);
#if 0
		for (i = SAMPLES/2; i < SAMPLES; ++i) {
			re[i] = re[i - SAMPLES + SAMPLES/2];
			im[i] = -im[i - SAMPLES + SAMPLES/2];
		}
#endif

		fft_rev(re, im, SAMPLES);
		for (i = 0; i < SAMPLES; ++i) {
			re[i] *= (double)2/SAMPLES;
			buf[i] = re[i] * INT16_MAX;
		}

		write(fd, buf, SAMPLES * sizeof (int16_t));
	}

	return 1;
}
