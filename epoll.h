#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS       1024

int ts_epoll_create(int flags);

void ts_epoll_add(int epoll_fd, int fd, struct epoll_event *event);

void ts_epoll_mod(int epoll_fd, int fd, struct epoll_event *event);

void st_epoll_del(int epoll_fd, int fd, struct epoll_event *event);

int ts_epoll_wait(int epoll_fd, struct epoll_event *events, int maxevents, int timeout);

#endif /* EPOLL_H */
