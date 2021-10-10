#include "generic.h"
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

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


void shift_buf(http_request_t *session, u_char *target) 
{
    int offset = target - session->buf;
    memmove(session->buf, target, offset);
    session->last -= offset;
    session->pos = session->buf;
    session->state = 0;
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