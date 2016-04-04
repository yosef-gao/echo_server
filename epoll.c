#include "epoll.h"
#include "dbg.h"

struct epoll_event *events;

int ts_epoll_create(int flags)
{
    /*
     * epoll_create()  
     * creates  an  epoll(7) instance.  Since Linux 2.6.8, the
     * size argument is ignored, but must be  greater  than  zero.
     * epoll_create1()
     * If  flags  is 0, then, other than the fact that the obsolete size argu‐
     * ment is dropped, epoll_create1() is the same  as  epoll_create(). 
     * */

    int fd = epoll_create1(flags);
    check_exit(fd > 0, "ts_epoll_create: epoll_create1");

    events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAXEVENTS);
    check_exit(events != NULL, "ts_epoll_create: malloc");

    return fd;
}

void ts_epoll_add(int epoll_fd, int fd, struct epoll_event *event)
{
    int rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
    check(rc == 0, "ts_epoll_add: epoll_ctl");

    return;
}

void ts_epoll_mod(int epoll_fd, int fd, struct epoll_event *event)
{
    int rc = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);
    check(rc == 0, "ts_epoll_mod: epoll_ctl");

    return;
} 

void ts_epoll_del(int epoll_fd, int fd, struct epoll_event *event)
{
    int rc = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, event);
    check(rc == 0, "ts_epoll_del: epoll_ctl");

    return;
}

int ts_epoll_wait(int epoll_fd, struct epoll_event *events, int maxevents, int timeout)
{
    int n = epoll_wait(epoll_fd, events, maxevents, timeout);
    check(n >= 0, "ts_epoll_wait: epoll_wait");

    return n;
}
