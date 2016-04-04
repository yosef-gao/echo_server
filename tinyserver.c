#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include "dbg.h"
#include "util.h"
#include "epoll.h"
#include "message.h"

extern struct epoll_event *events;

int main(int argc, char *argv[])
{
    int rc;
    /* Install signal handle for SIGPIPE
     * when a fd is clised by remote, writing to rhis fd will cause system send
     * SIGPIPE to this process, which default action is exit the program*/
    struct sigaction sa;
    memset((void *)&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) < 0)
    {
        log_err("install signal handler for SIGPIPE failed");
        exit(EXIT_FAILURE);
    }


    /* Initialize listening socket*/
    int listen_fd;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = 1;
    memset((void *)&clientaddr, 0, sizeof(clientaddr));

    listen_fd = open_listenfd(8080);
    rc = make_socket_non_blocking(listen_fd);
    check(rc == 0, "make_socket_non_blocking");

    /* Ceate epoll and listen to epoll*/
    int epoll_fd = ts_epoll_create(0);
    struct epoll_event event;

    ts_message_t *msg = (ts_message_t *)malloc(sizeof(ts_message_t));
    ts_init_message_t(msg, listen_fd, epoll_fd);

    event.data.ptr = (void *)msg;
    event.events = EPOLLIN | EPOLLET;
    ts_epoll_add(epoll_fd, listen_fd, &event);

    /* epoll_wait loop*/
    int i, n, fd;
    while (1)
    {
        n = ts_epoll_wait(epoll_fd, events, MAXEVENTS, 0);
        for (i = 0; i < n; ++i)
        {
            ts_message_t *msg = (ts_message_t *)events[i].data.ptr;
            fd = msg->fd;

            if (listen_fd == fd)
            {
                /* We have one or more incoming connections */
                int infd;
                while (1)
                {
                    infd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen);
                    if (infd < 0) /* error */
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            /* We have processed all incoming connections 
                             * more details: https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c */
                            break;
                        }
                        else
                        {
                            log_err("accpet");
                            break;
                        }
                    }

                    rc = make_socket_non_blocking(infd);
                    check(rc == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);

                    ts_message_t *msg = (ts_message_t *)malloc(sizeof(ts_message_t));
                    if (msg == NULL)
                    {
                        log_err("malloc ts_message_t");
                        break;
                    }

                    ts_init_message_t(msg, infd, epoll_fd);
                    event.data.ptr = (void *)msg;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    ts_epoll_add(epoll_fd, infd, &event);
                } // end of while accpet
            }
            else
            {
                /* We have one or more clients' messages to read */

                /* 
                 * EPOLLHUP:
                 * Stream socket peer closed connection, or shut down writing half
                 * of connection. (This flag is especially useful for writing simple
                 * code to detect peer shutdown when using Edge Triggered monitoring
                 * */

                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN)))
                {
                    log_err("epoll error fd: %d", fd);
                    close(fd);
                    continue;
                }

                //log_info("new data from fd %d:", fd);

                ts_handle_message(msg);
            }
        } // end of for
    } // end of while (1)

    exit(EXIT_SUCCESS);
}

/*
void echo_and_write_back(void *arg)
{
    int wc;
    ts_message_t *msg = (ts_message_t *)arg;

    // write the buffer to standard output
    fprintf(stdout, "client %d said: %s", msg->fd, msg->buf);

    // write back to client
    wc = writen(msg->fd, msg->buf, strlen(msg->buf));
    check(wc > 0, "echo_and_write_back");
}
*/
