#include "generic.h"
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

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