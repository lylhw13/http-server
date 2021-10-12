#include "generic.h"

#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <assert.h>

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

void shift_buf(http_request_t *session, u_char *position) 
{
    /* move the memory */
    int offset = position - session->buf;
    memmove(session->buf, position, offset);
    session->last -= offset;

    /* restart parse the line */
    session->pos = session->buf;
    session->state = 0;
}

void free_response(http_response_t *response)
{
    http_header_t *curr = response->headers;
    while (curr) {
        response->headers = curr->next;
        free(curr);
        curr = response->headers;       
    }   
    /* free body */
    free(response);
    response = NULL;
}

void free_response_list(http_response_t *response)
{
    http_response_t *curr = response;
    while(curr) {
        response = curr->next;
        free_response(curr);
        curr = response;
    }
    response = NULL;
}

void free_request(http_request_t *session)
{
    // free_response_list(session->responses);
}

char *status_text(int status)
{
    switch (status)
    {   
    case 200:
        return "OK";
    case 301:
        return "Moved Permanently";
    case 404:
        return "NOT Found";    
    default:
        break;
    }
    return "";
}

void write_header_to_buffer(http_request_t *session, http_response_t *response)
{
    int offset = 0;
    int nwrite = 0;
    http_header_t *header = response->headers;
    nwrite = sprintf(session->out_buf + offset, "HTTP/1.1 %d %s\r\n", response->status, status_text(response->status));
    offset += nwrite;

    while (header) {
        nwrite = sprintf(session->out_buf + offset, "%s: %s\r\n", header->key, header->value);
        offset += nwrite;
        header = header->next;
    }

    /* content-length */
    nwrite = sprintf(session->out_buf + offset, "Content-length: %d\r\n", response->body_length);
    offset += nwrite;

    if (session->keep_alive)
        nwrite = sprintf(session->out_buf + offset, "Connection: keep-alive\r\n");
    else
        nwrite = sprintf(session->out_buf + offset, "Connection: close\r\n");
    offset += nwrite;

    nwrite = sprintf(session->out_buf + offset, "\r\n");
    offset += nwrite;
    response->header_length = offset;

    // LOGD("write_header_to_buffer\n");
    // fprintf(stderr, session->out_buf, offset);
}

void send_response(http_request_t * session, http_response_t *curr_rsp)
{
    int nwrite = 0;
    while(1) {
        switch(curr_rsp->work_state){
            case WRITE_BEGIN:
            // LOGD("write begin\n");
                write_header_to_buffer(session, curr_rsp);
                curr_rsp->pos = 0;
                curr_rsp->work_state = WRITE_HEADER;
                //fall through
            case WRITE_HEADER:
            // LOGD("write_header\n");
                nwrite = write(session->fd, session->out_buf + curr_rsp->pos, curr_rsp->header_length - curr_rsp->pos);
                errno = 0;
                if (nwrite < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        return;
                }
                if (nwrite <= 0) {
                    session->state = SESSION_END;
                    return;
                }
                curr_rsp->pos += nwrite;
                if (curr_rsp->pos == curr_rsp->header_length) {
                    curr_rsp->pos = 0;
                    curr_rsp->work_state = WRITE_BODY;
                }
                break;

            case WRITE_BODY:
            // LOGD("write_body\n");
                nwrite = write(session->fd, curr_rsp->body + curr_rsp->pos, curr_rsp->body_length - curr_rsp->pos);
                // write(STDOUT_FILENO, curr_rsp->body + curr_rsp->pos, curr_rsp->body_length - curr_rsp->pos);
                errno = 0;
                if (nwrite < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        return;
                }
                if (nwrite <= 0) {
                    session->state = SESSION_END;
                    return;
                }
                curr_rsp->pos += nwrite;
                if (curr_rsp->pos == curr_rsp->body_length) {
                    // LOGD("body equal\n");
                    session->responses = curr_rsp->next;
                    free_response(curr_rsp);
                    curr_rsp->pos = 0;
                    curr_rsp->work_state = WRITE_BEGIN;
                    return;
                }
                break;

            default:
                return;
        }
    }
}

void do_respond(http_request_t *session)
{
    http_response_t *curr = session->responses;
    while(curr != NULL) {
        send_response(session, curr);
        curr = session->responses;
    }

    if (session->http_state == SESSION_END) {
        // fprintf(stderr, "respond end\n");
        // epoll_ctl(session->epfd, EPOLL_CTL_DEL, session->fd, NULL);
        close(session->fd); /* epoll will auto remove when fd is close */
        free(session);
    }
}

void set_http_response_status(http_response_t *response, int status)
{
    response->status = status;
}

void set_http_response_header(http_response_t *response, char const *key, char const *value)
{
    http_header_t *curr_header = (http_header_t*)malloc(sizeof(http_header_t));
    assert(curr_header != NULL);
    curr_header->key = key;
    curr_header->value = value;
    http_header_t *prev = response->headers;
    curr_header->next = prev;
    response->headers = curr_header;
}

void set_http_response_body(http_response_t *response, char const *body, int content_length)
{
    response->body = body;
    response->body_length = content_length;
}


void add_response(http_request_t *session, char *body)
{
    // LOGD("add_response\n");
    // char *body = "<html><head>this is header </head><body><h1>this is body</h1><body></html>";
    if (!body)
        body = "Hello, World!";
    http_response_t *cresponse = (http_response_t*)malloc(sizeof(http_response_t));
    if (cresponse == NULL)
        perror("error cmalloc");
    set_http_response_status(cresponse, 200);
    set_http_response_header(cresponse, "Content-type", "text/plain; charset=utf-8");
    set_http_response_body(cresponse, body, strlen(body));
    if (session->responses == NULL)
        session->responses = cresponse;
    else 
        session->responses->next = cresponse;
}

void check_url() 
{

}

void do_request(http_request_t *session) 
{
    int nread, nwrite;
    int parse_result;
    errno = 0;
    int len;

    parse_result = OK;

    /* OK to end session */
    if (session->state == PARSE_BEGIN && session->http_state == SESSION_END)
        return;

    if (session->buf + BUFSIZE <= session->last) /* full buf */ {
        return;
    }

    len =  (session->buf + BUFSIZE) - session->last;
    nread = read(session->fd, session->last, (session->buf + BUFSIZE) - session->last);
    // fprintf(stderr, "\nlen is %d, read is %d\n", (int)len, (int)nread);
    if (nread < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
    }

    if (nread <= 0) {
        // LOGE("connect close. clear\n");
        epoll_ctl(session->epfd, EPOLL_CTL_DEL, session->fd, NULL);
        close(session->fd);
        free(session);
        return;
    }

    session->last = session->last + nread;
    // fprintf(stderr, "last is %d\n", (int)(req->last - req->buf));
    // fprintf(stdout, "pos to last\n%.*s\n", (int)(req->last - req->pos), req->pos);

    while (session->pos < session->last) {
        switch (session->parse_state)
        {
        case PARSE_BEGIN:
            parse_result = http_parse_request_line(session);

            if (parse_result == AGAIN) {
                /* this case is that the shift can't help */
                if (session->request_start == session->buf) {
                    add_response(session, ngx_http_error_494_page);
                    session->http_state = SESSION_END;
                    return;
                }
                session->parse_state = PARSE_BEGIN;
                shift_buf(session, session->request_start);
                return;
            }

            // fprintf(stderr, "method: %.*s\n", (int)(session->method_end + 1 - session->request_start), session->request_start);
            // fprintf(stderr, "uri: %.*s\n", (int)(session->uri_end + 1 - session->uri_start), session->uri_start);
            check_url();
            session->parse_state = PARSE_HEADER;
            /* fall through */
        
        case PARSE_HEADER:
            parse_result = http_parse_header_lines(session);
            // fprintf(stderr, "\nparse result is %d\n", parse_result);
            if (parse_result == AGAIN) {
                session->parse_state = PARSE_HEADER;
                shift_buf(session, session->header_name_start);
                return;        
            }
            /* fall through */

        case PARSE_HEADER_DONE:
            if (session->method == HTTP_GET) {
                // fprintf(stderr, "parse header finish\n");
                add_response(session, NULL);
                session->parse_state = PARSE_BEGIN;
                if (session->keep_alive)
                    continue;

                if (session->pos == session->last) {
                    session->http_state = SESSION_END;
                    return;
                }
            }
            if (session->method == HTTP_POST) {
                //write the target file
                session->parse_state = PARSE_BODY;
            }
            /* fall through */

        case PARSE_BODY:
            nread = MIN(session->last - session->pos, session->content_length);
            nwrite = write(STDOUT_FILENO, session->pos, nread);
            LOGD("\nwrite %d\n", nwrite);

            session->content_length -= nwrite;
            session->pos += nwrite;

            if (session->last == session->pos)
                shift_buf(session, session->pos);

            if (session->content_length == 0) {
                session->parse_state = PARSE_BEGIN;
                add_response(session, NULL);
                return;
            }

            continue;
        
        default:
            break;
        }
    }

    return;

// close_out:
//     if (req->responses == NULL) {
//         fprintf(stderr, "response is null\n");
//         // epoll_ctl(req->epfd, EPOLL_CTL_DEL, req->fd, NULL);
//         close(req->fd);
//         free(req);
//     }
//     else {
//         // struct epoll_event event;
//         // event.data.ptr = req;
//         // event.events = EPOLLOUT;
//         // epoll_ctl(req->epfd, EPOLL_CTL_MOD, req->fd, &event);
//         fprintf(stderr, "shutdown read\n");
//         shutdown(req->fd, SHUT_RD);
//     }
//     return;
}