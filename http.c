#include "generic.h"

#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>


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
    /* free header */
    http_header_t *curr = response->headers;
    while (curr) {
        response->headers = curr->next;
        free(curr);
        curr = response->headers;       
    }   
    /* free body */
    if (response->body_memop == FREE_MALLOC)
        free(response->body);
    else if(response->body_memop == FREE_MMAP)
        munmap(response->body, response->body_length);
    
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
    free_response_list(session->responses);
}

void write_header_to_buffer(http_request_t *session, http_response_t *response)
{
    LOGD("begin write_header_to_buffer\n");
    int offset = 0;
    int nwrite = 0;
    http_header_t *header = response->headers;
    nwrite = sprintf(session->out_buf + offset, "HTTP/1.1 %s\r\n", response->status);
    offset += nwrite;

    while (header) {
        LOGD("%s: %s\r\n", header->key, header->value);
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

    LOGD("write_header_to_buffer\n");
    fprintf(stderr, session->out_buf, offset);
}

void send_response(http_request_t * session, http_response_t *curr_rsp)
{
    int nwrite = 0;
    while(1) {
        switch(curr_rsp->work_state){
            case WRITE_BEGIN:
            LOGD("write begin\n");
                write_header_to_buffer(session, curr_rsp);
                curr_rsp->pos = 0;
                curr_rsp->work_state = WRITE_HEADER;
                //fall through
            case WRITE_HEADER:
            LOGD("write_header\n");
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
            LOGD("write_body\n");
                nwrite = write(session->fd, curr_rsp->body + curr_rsp->pos, curr_rsp->body_length - curr_rsp->pos);
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
                    LOGD("body equal\n");
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
        close(session->fd); /* epoll will auto remove when fd is close */
        free(session);
    }
}

void set_http_response_status(http_response_t *response, char * status)
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

void set_http_response_body(http_response_t *response, char *body, int content_length)
{
    response->body = body;
    response->body_length = content_length;
}


void add_response(http_request_t *session, char *body, int memop)
{
    LOGD("add_response\n");
    // char *body = "<html><head>this is header </head><body><h1>this is body</h1><body></html>";
    if (!body)
        body = "Hello, World!";
    http_response_t *cresponse = (http_response_t*)malloc(sizeof(http_response_t));
    if (cresponse == NULL)
        perror("error cmalloc");
    set_http_response_status(cresponse, RSP_OK);
    set_http_response_header(cresponse, "Content-type", "text/plain; charset=utf-8");
    set_http_response_body(cresponse, body, strlen(body));
    cresponse->body_memop = memop;

    if (session->responses == NULL)
        session->responses = cresponse;
    else 
        session->responses->next = cresponse;
}

void add_sendfile_response(http_request_t *session, char *filename)
{
    LOGD("add_sendfile_response\n");
    int fd;
    struct stat st;
    size_t size;
    char *body;
    
    http_response_t *cresponse = (http_response_t*)malloc(sizeof(http_response_t));
    if (cresponse == NULL)
        perror("error cmalloc");
    set_http_response_status(cresponse, RSP_OK);
    set_http_response_header(cresponse, "Content-Disposition","attachment; filename=\"this_is_filename\"");

    body = MAP_FAILED;
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    errno = 0;
    if (!fstat(fd, &st)) {
        size = st.st_size;
        body = mmap(NULL, size, PROT_READ, MAP_PRIVATE,fd, 0);
    }
    close(fd);
    if (body == MAP_FAILED) {
        LOGD("%s mmap error %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    set_http_response_body(cresponse, body, size);
    cresponse->body_memop = FREE_MMAP;

    if (session->responses == NULL)
        session->responses = cresponse;
    else 
        session->responses->next = cresponse;
}

void add_special_response(http_request_t *session, char *rsp_state)
{
    http_response_t *cresponse;
    char *body;
    cresponse = (http_response_t*)malloc(sizeof(http_response_t));
    if (cresponse == NULL)
        perror("error cmalloc");
    body = (char *)malloc(512);
    if (body == NULL)
        perror("error malloc");

    set_http_response_status(cresponse, rsp_state);
    set_http_response_header(cresponse, "Content-type", "text/html; charset=utf-8");
    sprintf(body, "<html>" CRLF
                        "<head><title>%s</title></head>" CRLF
                        "<body>" CRLF
                        "<center><h1>%s</h1></center></body></html>" CRLF, rsp_state, rsp_state);
    set_http_response_body(cresponse, body, strlen(body));
    cresponse->body_memop = FREE_MALLOC;
    
    if (session->responses == NULL)
        session->responses = cresponse;
    else 
        session->responses->next = cresponse;
}

int check_url(http_request_t *session) 
{

    int res, length;
    u_char *p, *start, *end;
    char *filename;

    start = session->uri_start;
    end = session->uri_end;
    // if (session->args_start != NULL)
    //     end = session->args_start;

    if (*start != '/') {
        add_special_response(session, RSP_BAD_REQUEST);
        return -1;
    }
        // return -1;
    
    for (p = start; p != end; ++p) {
        if (*p == '.' && *(p+1) == '.') {/* double dot .. */
            add_special_response(session, RSP_BAD_REQUEST);
            return -1;
        }
    }

    if (1 == end - start) {
        add_special_response(session, RSP_OK);
        return 0;
    }
    length = end - start;
    filename = (char *)malloc(length + 2);
    filename[0] = '.';
    sprintf(filename + 1, "%.*s", length, start);
    filename[length + 1] = '\0';

    fprintf(stderr, "filename is %s\n", filename);

    if (access(filename, R_OK) == 0) {
        add_sendfile_response(session, filename);
        res = 0;
    }
    else {
        add_special_response(session, RSP_NOT_FOUND);
        res = -1;
    }
    free(filename);

    return res;
}

void do_request(http_request_t *session) 
{
    int nread, nwrite;
    int parse_result;
    errno = 0;
    int len;

    parse_result = OK;

    if (session->http_state == SESSION_END)
        return;

    if (session->buf + BUFSIZE <= session->last) /* full buf */ {
        return;
    }

    len =  (session->buf + BUFSIZE) - session->last;
    nread = read(session->fd, session->last, (session->buf + BUFSIZE) - session->last);
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

    while (session->pos < session->last) {
        switch (session->parse_state)
        {
        case PARSE_BEGIN:
            parse_result = http_parse_request_line(session);

            if (parse_result == AGAIN) {
                /* this case is that the shift can't help */
                if (session->request_start == session->buf) {
                    add_special_response(session, RSP_REQUEST_HEADER_FIELDS_TOO_LARGE);
                    session->http_state = SESSION_END;
                    return;
                }
                session->parse_state = PARSE_BEGIN;
                shift_buf(session, session->request_start);
                return;
            }

            if (parse_result < 0) {
                add_special_response(session, RSP_BAD_REQUEST);
                session->http_state = SESSION_END;
                return;
            }

            // fprintf(stderr, "method: %.*s\n", (int)(session->method_end + 1 - session->request_start), session->request_start);
            // fprintf(stderr, "uri: %.*s\n", (int)(session->uri_end + 1 - session->uri_start), session->uri_start);
            // fprintf(stderr, "args: %.*s\n", (int)(session->uri_end - session->args_start), session->args_start);
            // fprintf(stderr, "host: %.*s\n", (int)(session->host_end - session->host_start), session->host_start);
            // fprintf(stderr, "port: %.*s\n", (int)(session->port_end - session->port_start), session->port_start);
            // fprintf(stderr, "uri ext: %.*s\n", (int)(session->uri_end - session->uri_ext), session->uri_ext);

            session->parse_state = PARSE_HEADER;
            /* fall through */
        
        case PARSE_HEADER:
            parse_result = http_parse_headers(session);
            if (parse_result == AGAIN) {
                session->parse_state = PARSE_HEADER;
                shift_buf(session, session->header_name_start);
                return;        
            }
            /* fall through */

        case PARSE_HEADER_DONE:
            if (session->method == HTTP_GET) {
                if (check_url(session) != 0) {
                    session->http_state = SESSION_END;
                    return;
                }
                // add_response(session, NULL, FREE_NONE);
                // add_sendfile_response(session, NULL);
                // add_error_response(session, RSP_HTTP_VERSION_NOT_SUPPORTED);
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
                add_response(session, NULL, FREE_NONE);
                return;
            }

            continue;
        
        default:
            break;
        }
    }

    return;
}