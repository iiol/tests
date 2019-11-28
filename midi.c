#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/soundcard.h>

// 0 - 15
#define CHANNEL 0
#define KEYS 128

enum evtype {
	PRESS_KEY	= 0x90 + CHANNEL,
	RELEASE_KEY	= 0x80 + CHANNEL,
	PRESS_PEDAL	= 0x100,
	RELEASE_PEDAL	= 0x101,
};

// keys[21] is A0
// keys[33] is A1
// etc...
// keys[KEYS] is damper pedal
int keys[KEYS + 1];

int
main(int argc, char **argv)
{
	int fd;
	int i, rc;
	unsigned char buf[3];
	enum evtype evtype;
	int note, velocity;
	char *devname = "/dev/midi1";

	if (argc == 2)
		devname = argv[1];
	else if (argc > 2) {
		fprintf(stderr, "Usage: %s [devname]", argv[0]);
		return 1;
	}

	if ((fd = open(devname, O_RDWR, 0)) == -1) {
		perror("open()");
		return 1;
	}

	while (1) {
		rc = read(fd, buf, sizeof (buf));

		if (buf[0] == 0xB0 && buf[1] == 0x40) {
			evtype = (buf[2] == 0) ? RELEASE_PEDAL : PRESS_PEDAL;
			note = 0;
			velocity = 0x7F;
		}
		else {
			if (buf[1] >= KEYS) {
				fprintf(stderr, "Unknown key. event: 0x%x%x%x\n", buf[0], buf[1], buf[2]);
				continue;
			}

			evtype = buf[0];
			note = buf[1];
			velocity = buf[2];
		}

		switch (evtype) {
		case PRESS_KEY:
			keys[note] = velocity;
			buf[0] = PRESS_KEY;
			buf[1] = note;
			buf[2] = velocity;

			write(fd, buf, 3);
			printf("key pressed: note: %d, velocity: %d\n", note, velocity);

			note = note - 12;

			if (note >= 0 && keys[note] > 0) {
				if (velocity >= 50) {
					if (keys[note] != 0) {
						buf[0] = RELEASE_KEY;
						buf[1] = note;
						buf[2] = 64;
						write(fd, buf, 3);
					}

					velocity -= 50;
					keys[note] = velocity;
					buf[0] = PRESS_KEY;
					buf[1] = note;
					buf[2] = velocity;

					write(fd, buf, 3);
				}
			}

			break;

		case RELEASE_KEY:
			keys[note] = 0;
			buf[0] = RELEASE_KEY;
			buf[1] = note;
			buf[2] = 64;

			write(fd, buf, 3);
			printf("key released: note: %d\n", note);

			break;

		case PRESS_PEDAL:
			keys[KEYS] = 1;
			buf[0] = 0xB0;
			buf[1] = 0x40;
			buf[2] = 0x7F;

			write(fd, buf, 3);
			printf("Damper pedal pressed\n");

			break;

		case RELEASE_PEDAL:
			keys[KEYS] = 0;
			buf[0] = 0xB0;
			buf[1] = 0x40;
			buf[2] = 0x00;

			write(fd, buf, 3);
			printf("Damper pedal relessed\n");

			break;
		}
	}

	close(fd);

	return 0;
}
