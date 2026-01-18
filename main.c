#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>


#include "config.h"



int fd;

void emit(int fd, int type, int code, int val) {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    
    write(fd, &ie, sizeof(ie));
}


void handle_exit(int sig) {
    (void)sig;
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    exit(1);
}



int main() {
    signal(SIGINT, handle_exit);

    struct pollfd fds;
    fds.fd = open("/dev/input/event3", O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    fds.events = POLLIN;
    ioctl(fds.fd, EVIOCGRAB, (void*)1);


    // Setup uinput
    struct uinput_setup usetup;
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    for (int i = 0; i < KEY_MAX; i++) {
        if (i == KEY_RESERVED) continue;
        ioctl(fd, UI_SET_KEYBIT, i);
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x1234;
    strcpy(usetup.name, "ekb virtual keyboard");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);


    struct input_event ev;
    while (1) {
        int ret = poll(&fds, 1, -1);

        if (ret < 0) {
            perror("poll");
            break;
        }

        if (fds.revents & POLLIN) {
            while (read(fds.fd, &ev, sizeof(ev)) > 0) {
                if (ev.type == EV_SYN) {
                    emit(fd, ev.type, ev.code, ev.value);
                    continue;
                }
                if (ev.type == EV_KEY) {
                    for (size_t j = 0; j < (sizeof(map) / sizeof(map[0])); j++) {
                        if (map[j].cmd == NULL && map[j].mod1 == 0) {
                            if (ev.code == map[j].key) {
                                emit(fd, ev.type, map[j].remap, ev.value);
                                break;
                            } 
                        } else if (map[j].cmd != NULL && ev.code == map[j].key && ev.value == 1) {
                            posix_spawn_file_actions_t actions;
                            posix_spawn_file_actions_init(&actions);
                            posix_spawn_file_actions_addclose(&actions, fds.fd);
                            posix_spawn_file_actions_addclose(&actions, fd);
                            posix_spawn_file_actions_addclose(&actions, 0);
                            posix_spawn_file_actions_addclose(&actions, 1);
                            posix_spawn_file_actions_addclose(&actions, 2);
                            char *argv[] = {"sh", "-c", map[j].cmd, NULL};
                            extern char **environ;
                            pid_t pid;
                            int status = posix_spawnp(&pid, "sh", &actions, NULL, argv, environ);
                            if (status == 0) {
                            } else {
                                fprintf(stderr, "posix_spawn: %s\n", strerror(status));
                            }
                            posix_spawn_file_actions_destroy(&actions);
                            break;
                        } else if (map[j].key == ev.code) {
                            emit(fd, EV_KEY, map[j].mod1, ev.value);
                            if (map[j].remap != 0) {
                                emit(fd, EV_KEY, map[j].mod2, ev.value);
                            }
                            emit(fd, EV_KEY, map[j].remap, ev.value);

                            emit(fd, EV_SYN, SYN_REPORT, 0);
                            break;
                        }
                    }
                } else {
                    emit(fd, ev.type, ev.code, ev.value);
                }
            }
        }
    }
}
