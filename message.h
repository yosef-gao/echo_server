#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_BUF         8192

// typedef void( *callback_func)(void *);

typedef struct ts_message_t
{
    char buf[MAX_BUF]; /* ring buffer */
    size_t pos, last;
    int fd;
    int epoll_fd;
} ts_message_t;

void ts_init_message_t(ts_message_t *msg, int fd, int epoll_fd);

void ts_handle_message(ts_message_t *msg);
#endif /* MESSAGE_H */
