#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "util.h"
#include "dbg.h"

int open_listenfd(int port)
{
    if (port <= 0)
    {
        port = 8080;
    }
    else if (port > 0 && port < 1024)
    {
        /* Check if is root user*/
        if (getuid() != 0);
            return -1;
    }

    int listenfd, optval = 1;
    struct sockaddr_in servaddr;

    /* Create a socket descriptor*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Set socket option */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0)
    return -1;

    bzero((void *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return -1;

    /* listen */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;

    return listenfd;
}

int make_socket_non_blocking(int fd)
{
    int flags, ret;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        log_err("fcntl get");
        return -1;
    }

    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1)
    {
        log_err("fcntl set");
        return -1;
    }

    return 0;
}

ssize_t writen(int fd, const void *buff, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = buff;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return (-1); /* error */
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}
