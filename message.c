#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/epoll.h>
#include "epoll.h"
#include "util.h"
#include "dbg.h"
#include "message.h"

void ts_init_message_t(ts_message_t *msg, int fd, int epoll_fd)
{
    memset(&msg->buf, 0, sizeof(MAX_BUF));
    msg->pos = msg->last = 0;
    msg->fd = fd;
    msg->epoll_fd = epoll_fd;
}

static ssize_t find_last_cr_lf(char *buf, size_t start, size_t end);

void ts_handle_message(ts_message_t *msg)
{
    int fd = msg->fd;
    ssize_t n;
    size_t remain_size;
    ssize_t i;
    char *plast;

    for (;;)
    {
        plast = &msg->buf[msg->last % MAX_BUF];
        /* msg->buf is a ring buffer 
         * if msg->last > msg->pos then remain_size is (MAX_BUF - msg->last % MAX_BUF)
         * if msg->last < msg->pos then remain_size is (MAX_BUF - (msg->last - msg->pos))
         * */
        remain_size = MIN(MAX_BUF - (msg->last - msg->pos) - 1, MAX_BUF - msg->last % MAX_BUF); 

        n = read(fd, plast, remain_size);
        check(msg->last - msg->pos < MAX_BUF, "buffer overflow");

        if (n == 0)
        {
            /* EOF */
            log_err("read err, and errno = %d", errno);
            goto err;
        }

        if (n < 0)
        {
            if (errno != EAGAIN)
            {
                log_err("read err, and errno = %d", errno);
                goto err;
            }
            break;
        }

        msg->last += n;
        check(msg->last - msg->pos < MAX_BUF, "buffer overflow");
    } // end for(;;)

    //log_info("ready to parse buffer");
    /* find first \r\n */
    // uint16_t cr_lf = ('\r' << 8 | '\n');
    i = find_last_cr_lf(msg->buf, msg->pos, msg->last);
    if (i >= 0 && (i + 1 - msg->pos) > 0)
    {
        char echo_msg_buf[MAX_BUF] = {0};
        strncpy(echo_msg_buf, &msg->buf[msg->pos], i + 1 - msg->pos); /* make a copy of buffer */
        /* write the buffer to standard outout */
        fprintf(stdout, "client %d said: %s", msg->fd, echo_msg_buf);
        /* write back to client */
        if (writen(msg->fd, echo_msg_buf, strlen(echo_msg_buf)) < 0)
            goto err;
        msg->pos = i + 1;
        if (msg->pos > MAX_BUF && msg->last > MAX_BUF)
        {
            msg->pos -= MAX_BUF;
            msg->last -= MAX_BUF;
        }
    }

    /* reset events */
    struct epoll_event event;
    event.data.ptr = (void *)msg;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    ts_epoll_mod(msg->epoll_fd, msg->fd, &event);
    return;

err:
    close(fd);
    free(msg);
}

static ssize_t find_last_cr_lf(char *buf, size_t start, size_t end)
{
    size_t i = end;
    for (; i != start; --i)
    {
        if (buf[i % MAX_BUF] == '\n' && buf[(i - 1) % MAX_BUF] == '\r')
        {
            return i;
        }
    }

    return -1;
}
