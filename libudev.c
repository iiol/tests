#include <stdio.h>
#include <string.h>
#include <libudev.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

int
main(void)
{
	int ret;
	int udev_fd;
	fd_set fds;
	const char *path, *status;
	struct udev *udev;
	struct udev_device *udev_dev;
	struct udev_enumerate *udev_enum;
	struct udev_list_entry *udev_list, *udev_list_entry;
	struct udev_monitor *udev_mon;


	if ((udev = udev_new()) == NULL) {
		fprintf(stderr, "Can not create udev object.\n");
		return 1;
	}

	if ((udev_enum = udev_enumerate_new(udev)) == NULL ) {
		fprintf(stderr, "Can not create unumerate object\n");
		return 1;
	}

	udev_enumerate_add_match_subsystem(udev_enum, "drm");
	udev_enumerate_scan_devices(udev_enum);

	if ((udev_list = udev_enumerate_get_list_entry(udev_enum)) == NULL) {
		fprintf(stderr, "Can not get dev list\n");
		return 1;
	}

	printf("exists:\n");
	udev_list_entry_foreach(udev_list_entry, udev_list) {
		path = udev_list_entry_get_name(udev_list_entry);
		udev_dev = udev_device_new_from_syspath(udev, path);
		status = udev_device_get_sysattr_value(udev_dev, "status");

		if (status == NULL || strcmp(status, "disconnected") == 0) {
			udev_device_unref(udev_dev);
			continue;
		}

		printf("%s\n", udev_device_get_sysname(udev_dev));

		udev_device_unref(udev_dev);
	}

	udev_mon = udev_monitor_new_from_netlink(udev, "udev");

	udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "drm", NULL);
	udev_monitor_enable_receiving(udev_mon);
	udev_fd = udev_monitor_get_fd(udev_mon);

	while (1) {
		FD_ZERO(&fds);
		FD_SET(udev_fd, &fds);

		ret = select(udev_fd + 1, &fds, NULL, NULL, NULL);
		if (ret <= 0)
			break;

		if (FD_ISSET(udev_fd, &fds))
			udev_dev = udev_monitor_receive_device(udev_mon);
	}

	return 0;
}
