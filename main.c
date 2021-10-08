#include "generic.h"

#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 64


void error(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

static struct option const longopts[] = 
{
    {"listen",  no_argument,       0, 'l'},   
    {"aes",     no_argument,       0, 's'},
};

void usage(int state)
{
    fprintf(stderr, 
        "usage: ncs [-s passwd] -l port\n"
        "       ncs [-s passwd] host port\n");
    exit(state);
}

void setnonblocking(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        return;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return;
}

int create_and_bind(const char* port) 
{
    struct addrinfo hints, *result, *rp;
    int ecode;
    int listenfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((ecode = getaddrinfo(NULL, port, &hints, &result))) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listenfd == -1)
            continue;

        int opt = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
            perror("setsockopt");

        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(listenfd);
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        error("Could not bind");
    }
    return listenfd;
}

void do_request(http_request_t *req) 
{
    return;
}

/* single process */
int main(int argc, char *argv[])
{    
    char *port = "33333";
    int listenfd;
    int currfd;
    int epfd;

    struct sockaddr_storage cliaddr;
    socklen_t cliaddr_len;
    int connfd;
    struct epoll_event event, *events;
    int nr_events, i;
    int nread;
    char buf[BUFSIZ];

    listenfd = create_and_bind(port);
    LOGD("listen fd %d\n", listenfd);
    if (listenfd < 0) 
        error("create and bind");

    if (listen(listenfd, SOMAXCONN) < 0)
        error("listen");
    setnonblocking(listenfd);
    
    epfd = epoll_create1(0);
    if (epfd < 0)
        error("epoll_create1");


    /* Add listen to epoll */
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLOUT;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event) < 0)
        error("epoll_ctl");

    events = malloc(sizeof (struct epoll_event) * MAX_EVENTS);
    if (!events)
        error("malloc");

    while (1) {
        nr_events = epoll_wait(epfd, events, MAX_EVENTS, 0);
        if (nr_events < 0) {
            error("epoll_wait");
            free(events);
            return -1;
        }

        for (i = 0; i < nr_events; ++i) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);   /* does this would block */

                LOGD("connect %d\n", connfd);
                if (connfd > 0) {
                    setnonblocking(connfd);

                    http_request_t *ptr = malloc(sizeof(http_request_t));
                    if (ptr == NULL)
                        continue;

                    ptr->fd = connfd;

                    // event.data.fd = connfd;
                    event.data.ptr = ptr;
                    event.events = EPOLLIN;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event) < 0)
                        error("epoll_ctl");

                }
                continue;
            }
            else {
                /* producer and consumer */
                if (events[i].events & EPOLLIN) {
                    // LOGE("epoll wait %d", events[i].data.fd);
                    http_request_t *ptr = events[i].data.ptr;
                    errno = 0;
                    nread = read(ptr->fd, ptr->buf, BUFSIZ);
                    
                    if (nread < 0) 
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            continue;
                    
                    if (nread <= 0) {
                        LOGE("close fd %d\n", events[i].data.fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(connfd);
                    }

                    if (nread > 0) {
                        write(STDOUT_FILENO, buf, nread);
                        ptr->last += nread;
                    }
                }
            }
        }   /* end for */
    }   /* end while */

    free(events);
    return 0;
}