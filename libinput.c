#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <xkbcommon/xkbcommon.h>
#include <libinput.h>
#include <libudev.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

int
open_restricted(const char *path, int flags, void *user_data)
{
	int ret;

	ret = open(path, flags);
	fprintf(stderr, "open: fd: %d, path: %s\n", ret, path);

	return ret;
}

void
close_restricted(int fd, void *user_data)
{
	printf("close: fd: %d\n", fd);
	close(fd);
}

int
event_handle(struct libinput *input)
{
	struct libinput_event *ev;
	struct libinput_event_keyboard *kev;
	uint32_t time, key, state;

	libinput_dispatch(input);

	while ((ev = libinput_get_event(input)) != NULL) {
		struct libinput_device *dev;
		int evtype;

		evtype = libinput_event_get_type(ev);

		switch (evtype) {
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			kev = libinput_event_get_keyboard_event(ev);
			time = libinput_event_keyboard_get_time(kev);
			key = libinput_event_keyboard_get_key(kev);
			state = libinput_event_keyboard_get_key_state(kev);
			printf("%d %d\n", key, state);
			break;
		default:
			printf("unknown event type: %d\n", evtype);
		}
	}

	return 0;
}

int
main(void)
{
	fd_set fds;
	int ret, fd;
	struct timeval tv;
	struct udev *udev;
	struct libinput *input;
	struct libinput_interface iface = {
		.open_restricted = open_restricted,
		.close_restricted = close_restricted,
	};

	udev = udev_new();
	assert(udev && "udev_new()");

	input = libinput_udev_create_context(&iface, NULL, udev);
	assert(input && "libinput_udev_create_context()");

	ret = libinput_udev_assign_seat(input, "seat0");
	assert(ret != -1 && "libinput_udev_assign_seat");

	fd = libinput_get_fd(input);
	printf("libinput fd: %d\n", fd);

	while (1) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		ret = select(fd+1, &fds, NULL, NULL, &tv);
		if (ret == -1) {
			perror("select()");
			return 1;
		}
		else if (!ret)
			continue;

		event_handle(input);
	}

	return 0;
}
