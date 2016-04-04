#ifndef UTIL_H
#define UTIL_H

#define LISTENQ         1024

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int open_listenfd(int port);

int make_socket_non_blocking(int fd);

ssize_t writen(int fd, const void *buff, size_t n);
#endif /* UTIL_H */
