#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <wayland-server.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#define DEPTH 24
#define BPP 32

#if 0
int
test(void)
{
        int fd;
        char dri_path[] = "/dev/dri/card0";
        struct wl_dispaly *display;
        struct wl_event_loop *event_loop;

        printf("Using wyland version: %s\n", WAYLAND_VERSION);

        if ((fd = open(dri_path, O_RDWR)) < 0) {
                printf("File opening error.\n");
                return 1;
        }

        if ((display = wl_display_create()) == NULL) {
                printf("Display creating error.\n");
                return 1;
        }

        if ((event_loop = wl_event_loop_create()) == NULL) {
                printf("Event loop creating error.\n");
                return 1;
        }

        wl_display_destroy(display);
        wl_event_loop_destroy(event_loop);

        return 0;
}
#endif

struct dev {
        int fd;
        uint32_t *buf;
        uint32_t connid, encid, crtcid, fbid;
        uint32_t w, h;
        uint32_t pitch, size, handle;
        drmModeModeInfo mode;
} dev;

unsigned int seed;

void
setcolor(uint32_t *buf, uint32_t color, int h, int w)
{
        int i, j;

        for (i = 0; i < h; ++i)
                for (j = 0; j < w/2; ++j)
                        *(buf + i*w + j) = color +
                            ((rand_r(&seed)%0xF0) << 16) +
                            ((rand_r(&seed)%0xF0) << 8) +
                            ((rand_r(&seed)%0xF0));
}

int
main(void)
{
        int i, j;
        char dripath[] = "/dev/dri/card0";

        drmModeRes *res;
        drmModeConnector *conn;
        drmModeEncoder *enc;

        struct drm_mode_create_dumb creq;
        struct drm_mode_map_dumb mreq;

#if 1
        if ((dev.fd = open(dripath, O_RDWR)) < 0) {
                fprintf(stderr, "open() error\n");
                return 1;
        }
#else
        if ((dev.fd = drmOpen(dripath, "asdf")) < 0) {
                printf("Open error\n");
                return 1;
        }
#endif

#if 1   // Check dumb_buf support
        uint64_t has_dumb;
        int flags;

        if ((flags = fcntl(dev.fd, F_GETFD)) < 0
                        || fcntl(dev.fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
                printf("fcntl FD_CLOEXEC error\n");
                return 1;
        }

        if (drmGetCap(dev.fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || has_dumb == 0) {
                printf("drmGetCap() error\n");
                return 1;
        }
#endif

        if ((res = drmModeGetResources(dev.fd)) == NULL) {
                fprintf(stderr, "drmModeGetResources() error\n");
                return 1;
        }

#if 1   // Debug
        printf("count fbs: %d\n", res->count_fbs);
        printf("count crtcs: %d\n", res->count_crtcs);
        printf("count connectors: %d\n", res->count_connectors);
        printf("min h: %d, max h: %d\n", res->min_height, res->max_height);
        printf("min w: %d, max w: %d\n", res->min_width, res->max_width);
#endif

        printf("Connectors list:\n\n");
        for (i = 0; i < res->count_connectors; ++i) {
                conn = drmModeGetConnector(dev.fd, res->connectors[i]);

                printf("Conn id: %d\n", conn->connector_id);
                printf("\tEncoder id: %d\n", conn->encoder_id);
                printf("\tCount modes: %d\n", conn->count_modes);

                for (j = 0; j < conn->count_modes; ++j) {
                        printf("\t%d)\n", j + 1);
                        printf("\t\tWidth : %d\n", conn->modes[j].hdisplay);
                        printf("\t\tHeight: %d\n", conn->modes[j].vdisplay);
                }

                if (conn != NULL && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
                        if ((enc = drmModeGetEncoder(dev.fd, conn->encoder_id)) == NULL)
                                fprintf(stderr, "Get encoder error\n");

                        printf("\tCRTC id: %d\n", enc->crtc_id);

                        dev.connid = conn->connector_id;
                        dev.encid = conn->encoder_id;
                        dev.crtcid = enc->crtc_id;

                        dev.mode = conn->modes[0];
                        dev.w = conn->modes[0].hdisplay + 10; //What is it?
                        dev.h = conn->modes[0].vdisplay;

                        drmModeFreeEncoder(enc);
                }

                drmModeFreeConnector(conn);

                putchar('\n');
        }

        drmModeFreeResources(res);

        memset(&creq, 0, sizeof (struct drm_mode_create_dumb));
        creq.width = dev.w;
        creq.height = dev.h;
        creq.bpp = BPP;

        if (drmIoctl(dev.fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
                printf("drmIoctl() error\n");
                return 1;
        }

        printf("Pitch: %d\n", creq.pitch);
        printf("Pitch/3: %d\n", creq.pitch/3);
        printf("Size: %lld\n", creq.size);
        printf("Handle: %d\n", creq.handle);

        dev.pitch = creq.pitch;
        dev.size = creq.size;
        dev.handle = creq.handle;

        if (drmModeAddFB(dev.fd, dev.w, dev.h, DEPTH, BPP, dev.pitch,
                         dev.handle, &dev.fbid)) {
                printf("drmModeAddFB() error\n");
                return 1;
        }

        printf("DEBUG: fd: %d, w: %d, h: %d, pitch: %d, handle: %d, fbid: %d\n",
               dev.fd, dev.w, dev.h, dev.pitch, dev.handle, dev.fbid);

#if 0   // Debug
        if ((res = drmModeGetResources(dev.fd)) == NULL) {
                fprintf(stderr, "drmModeGetResources() error\n");
                return 1;
        }

        printf("count fbs: %d\n", res->count_fbs);
        printf("count crtcs: %d\n", res->count_crtcs);
        printf("count connectors: %d\n", res->count_connectors);
        printf("fbid: %d\nfbid2: %d\n", res->fbs[0], dev.fbid);

        drmModeFreeResources(res);
#endif

        memset(&mreq, 0, sizeof(struct drm_mode_map_dumb));
        mreq.handle = dev.handle;

        if (drmIoctl(dev.fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
                printf("drmIoctl() error\n");
                return 1;
        }

        dev.buf = mmap(NULL, dev.size, PROT_READ | PROT_WRITE, MAP_SHARED, dev.fd, mreq.offset);
        if (dev.buf == MAP_FAILED) {
                printf("mmap() error\n");
        }

        drmModeGetCrtc(dev.fd, dev.crtcid);
        if (drmModeSetCrtc(dev.fd, dev.crtcid, dev.fbid, 0, 0,
                           &dev.connid, 1, &dev.mode)) {
                printf("drmModeSetCrtc() error\n");
                return 1;
        }

#if 1   // test drmModeSetPlane()
        drmModePlaneResPtr planes;
        drmModePlanePtr plane;
        int ret;

        planes = drmModeGetPlaneResources(dev.fd);

#if 0   // Add new FB
        uint32_t * fbuf;
        uint32_t fb2id;
        if (drmModeAddFB(dev.fd, dev.w, dev.h, DEPTH, BPP, dev.pitch,
                         dev.handle, &fb2id)) {
                printf("drmModeAddFB() error\n");
                return 1;
        }
        printf("ib2id: %d\n", fb2id);

        memset(&mreq, 0, sizeof(struct drm_mode_map_dumb));
        mreq.handle = dev.handle;

        if (drmIoctl(dev.fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
                printf("2 drmIoctl() error\n");
                return 1;
        }

        fbuf = mmap(NULL, dev.size, PROT_READ | PROT_WRITE, MAP_SHARED, dev.fd, mreq.offset);
        if (dev.buf == MAP_FAILED) {
                printf("2 mmap() error\n");
        }

        printf("count planes: %d\n", planes->count_planes);
        for (i = 0; i < planes->count_planes; ++i)
                printf("\t%d) %d\n", i + 1, planes->planes[i]);

#endif
#if 1
        ret = drmModeSetPlane(dev.fd, planes->planes[0], dev.crtcid, dev.fbid, 0,
                              //crtc_x, crtc_y, crtc_w, crtc_h
                              dev.w/2, dev.h/2, dev.w/2, dev.h/2,
                              //src_x, src_y, src_w, src_h
                              (dev.w/4) << 16, (dev.h/4) << 16, (dev.w/2) << 16, (dev.h/2) << 16
                             ); // Sad smile

        if (ret) {
                if (ret == -EINVAL) {
                        printf("drmModeSetPlane(): plane_id of crtc_id is invalid\n");
                }
                else {
                        errno = -ret;
                        perror("drmModeSetPlane()");
                }

                return 1;
        }
#endif
#endif

        uint8_t r, g, b;

        while (1) {
                for (r = 0xFF; r != 0x00; --r, ++g) {
                        setcolor(dev.buf, (r << 16) + (g << 8), dev.h, dev.w);
                        usleep(1);
                }

                for (g = 0xFF; g != 0x00; --g, ++b) {
                        setcolor(dev.buf, (g << 8) + b, dev.h, dev.w);
                        usleep(1);
                }

                for (b = 0xFF; b != 0x00; --b, ++r) {
                        setcolor(dev.buf, (r << 16) + b, dev.h, dev.w);
                        usleep(1);
                }
        }

        close(dev.fd);

        return 0;
}
