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

void shift_buf(http_request_t *session, u_char *target) 
{
    int offset = target - session->buf;
    memmove(session->buf, target, offset);
    session->last -= offset;
    session->pos = session->buf;
    session->state = 0;
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

        if (setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)))
            perror("setsockopt keep alive");

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

void do_response(http_request_t *session)
{
    int offset;
    char buf[BUFSIZE];
    char *header = "HTTP/1.1 200 OK\r\nContent-type: text/html; charset=utf-8\r\nConnection: keep-alive\r\nContent-length: %d\r\n\r\n";
    char *content = "<html><head>this is header </head><body><h1>this is body</h1><body></html>";
    offset = sprintf(buf, header, strlen(content));
    strcat(buf + offset, content);
    write(session->fd, buf, offset + strlen(content));
}

int parse_request_line(http_request_t *req, int nread) 
{
    int res;
    req->pos = req->buf;
    req->last = req->buf + nread;
    res = http_parse_request_line(req);

    fprintf(stderr, "method: %.*s\n", (int)(req->method_end + 1 - req->request_start), req->request_start);
    fprintf(stderr, "uri: %.*s\n", (int)(req->uri_end + 1 - req->uri_start), req->uri_start);
    return res;
}


void do_request(http_request_t *req) 
{
    int nread;
    int parse_result;
    errno = 0;
    int len;

    parse_result = OK;

    if (req->buf + BUFSIZE <= req->last) /* full buf */ {
        return;
    }

    len =  (req->buf + BUFSIZE) - req->last;
    nread = read(req->fd, req->last, (req->buf + BUFSIZE) - req->last);
    // fprintf(stderr, "\nlen is %d, read is %d\n", (int)len, (int)nread);
    if (nread < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
    }

    if (nread <= 0) {
        LOGE("connect close. clear\n");
        epoll_ctl(req->epfd, EPOLL_CTL_DEL, req->fd, NULL);
        close(req->fd);
        free(req);
        return;
    }

    req->last = req->last + nread;
    // fprintf(stderr, "last is %d\n", (int)(req->last - req->buf));
    // fprintf(stdout, "pos to last\n%.*s\n", (int)(req->last - req->pos), req->pos);

    while (req->pos < req->last) {
        switch (req->session_state)
        {
        case PARSE_BEGIN:
            parse_result = http_parse_request_line(req);

            if (parse_result == AGAIN) {
                req->session_state = PARSE_BEGIN;
                shift_buf(req, req->request_start);
                return;
            }

            fprintf(stderr, "method: %.*s\n", (int)(req->method_end + 1 - req->request_start), req->request_start);
            fprintf(stderr, "uri: %.*s\n", (int)(req->uri_end + 1 - req->uri_start), req->uri_start);
            // check url
            req->session_state = PARSE_HEADER;
            // fall through
        
        case PARSE_HEADER:
            parse_result = http_parse_header_lines(req);
            fprintf(stderr, "\nparse result is %d\n", parse_result);
            if (parse_result == AGAIN) {
                req->session_state = PARSE_HEADER;
                shift_buf(req, req->header_name_start);
                return;        
            }

            if (req->method == HTTP_GET) {
                ;   // return the target file
                req->session_state = PARSE_BEGIN;
                // memmove(req->buf, req->pos, req->last - req->pos);   /* which is better, ring buffer or memove */
                fprintf(stderr, "parse header finish\n");
                do_response(req);
                if (req->keep_alive)
                    continue;
                else
                    goto close_out;
                // continue;
                // return;
            }
            // if (req->method == HTTP_POST) {
            //     ;   //write the target file
            //     req->session_state = PARSE_BODY;
            // }
            // fall through

        // case PARSE_BODY:
        //     memmove(req->buf, req->pos, req->last - req->pos);   /* which is better, ring buffer or memove */

        //     req->session_state = PARSE_BEGIN;
        //     req->last -= (req->pos - req->buf);
        //     req->pos = req->buf;
        //     return;
        
        default:
            break;
        }
    }

    return;

close_out:
    epoll_ctl(req->epfd, EPOLL_CTL_DEL, req->fd, NULL);
    close(req->fd);
    free(req);
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

                LOGD("listen: connect %d\n", connfd);
                if (connfd > 0) {
                    int opt;
                    setnonblocking(connfd);
                    // if (setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)))
                    //     perror("setsockopt keep alive");

                    http_request_t *ptr = (http_request_t *)malloc(sizeof(http_request_t));
                    if (ptr == NULL)
                        continue;

                    memset(ptr, 0, sizeof(http_request_t));
                    /* init session */
                    ptr->fd = connfd;
                    ptr->pos = ptr->buf;
                    ptr->last = ptr->buf;

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
                    do_request(events[i].data.ptr);
                }
            }
        }   /* end for */
    }   /* end while */

    free(events);
    return 0;
}