#ifndef GENERIC_H
#define GENERIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 512

#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

typedef unsigned char u_char;
typedef unsigned int uint_t;
typedef int int_t;
typedef unsigned int uint32_t;


typedef struct http_request http_request_t;

typedef struct http_buf http_buf_t;


typedef struct {
    size_t len;
    u_char *data;
} str_t;


#define LF (u_char)'\n'
#define CR (u_char)'\r'
#define CRLF "\r\n"

/* parse request line */

#define HTTP_UNKNOWN                   0x00000001
#define HTTP_GET                       0x00000002
#define HTTP_POST                      0x00000008

#define PARSE_BEGIN 0
#define PARSE_HEADER 1
#define PARSE_BODY 2

#define HTTP_PARSE_HEADER_DONE         1

#define HTTP_CLIENT_ERROR              10
#define HTTP_PARSE_INVALID_METHOD      10
#define HTTP_PARSE_INVALID_REQUEST     11
#define HTTP_PARSE_INVALID_VERSION     12
#define HTTP_PARSE_INVALID_09_METHOD   13

#define HTTP_PARSE_INVALID_HEADER      14

#define OK 0
#define ERROR -1
#define AGAIN -2

#define HTTP_LC_HEADER_LEN 32

// GET /rfc/rfc2616.txt HTTP/2

// parse state

/*
 * POST method 
 * parse_begin
 * parse_request_in
 * parse_request_finish
 * parse_headers_in
 * parse_headers_finish
 * parse_body_in
 * parse_body_finish
 */
// #define PARSE_REQUEST
// #define PARSE_HEADERS
// #define PRASE_BODY

/*
 * GET method
 * parse_begin
 * parse_request_in
 * parse_request_finish
 * respond_in
 * respond_finish
 */

// #define PARSE_HEADER_IN 1
// #define PARSE_REQUEST_IN 2
// #define PARSE_REQUEST_FINISH 3
// #define RESPOND_IN 4
// #define RESPOND_FINISH 5

#define SESSION_INIT 0 
#define SESSION_READ 1

#define MIN(a, b) ({\
    typeof(a) _a = a; \
    typeof(b) _b = b;  \
    _a < _b?_a:_b;})

#define MAX(a, b) ({\
    typeof(a) _a = a; \
    typeof(b) _b = b;  \
    _a > _b?_a:_b;})


struct http_request {
    int fd;
    int epfd;
    u_char buf[BUFSIZE];
    u_char *pos;
    u_char *last;
    
     /* used to parse HTTP headers */
    uint_t                        state;

    uint_t session_state;

    uint_t                        header_hash;
    uint_t                        lowcase_index;
    u_char                            lowcase_header[HTTP_LC_HEADER_LEN];

    u_char                           *header_name_start;
    u_char                           *header_name_end;
    u_char                           *header_start;
    u_char                           *header_end;

    /*
     * a memory that can be reused after parsing a request line
     * via ngx_http_ephemeral_t
     */

    /* used to parse HTTP request line */

    u_char                           *uri_start;
    u_char                           *uri_end;
    u_char                           *uri_ext;
    u_char                           *args_start;
    u_char                           *request_start;    /* request line start */
    u_char                           *request_end;      /* request line end */
    u_char                           *method_end;       /* method end */
    u_char                           *schema_start;
    u_char                           *schema_end;
    u_char                           *host_start;
    u_char                           *host_end;
    u_char                           *port_start;
    u_char                           *port_end;

    unsigned                          http_minor:16;
    unsigned                          http_major:16;

    uint_t                        method;
    uint_t                        http_version;

    unsigned                          count:16;
    unsigned                          subrequests:8;
    unsigned                          blocked:8;

    unsigned                          aio:1;

    unsigned                          http_state:4;
    unsigned keep_alive:1;
    uint_t content_length;

    /* URI with "/." and on Win32 with "//" */
    unsigned                          complex_uri:1;

    /* URI with "%" */
    unsigned                          quoted_uri:1;

    /* URI with "+" */
    unsigned                          plus_in_uri:1;

    /* URI with empty path */
    unsigned                          empty_path_in_uri:1;

    unsigned                          invalid_header:1;

    str_t                         http_protocol;


};

struct http_buf {
    u_char *pos;
    u_char *last;
};

extern int http_parse_request_line(http_request_t *r);
extern int_t ngx_http_parse_header_line(http_request_t *r, uint_t allow_underscores);

static void print_str(char *start, char *end)
{
    char *p;
    for (p = start; p != end; ++p)
        fputc(*p, stderr);

    fputs("", stderr);
}

static int str_to_buf_lowcase(char *buf, const char *start, const char *end, int n)
{
    int len, i;
    u_char c;

    len = MAX(0, MIN(n - 1, end - start));

    for (i = 0; i< len; ++i) {
        c = start[i];

        if (c >= 'A' && c<= 'Z')
           c =  c - 'A' + 'a';

        buf[i] = c;
    }
    buf[len] = '\0';
    return len;
}

static int http_parse_header_lines(http_request_t *r) 
{
    int_t  res;
    int i;

    for (;;) {
        res = ngx_http_parse_header_line(r, 1);
        if (res == HTTP_PARSE_INVALID_HEADER) {
            fprintf(stderr, "invalid");
            return res;
        }
        if (res == HTTP_PARSE_HEADER_DONE) {
            fputs("DONE......................", stderr);
            // print_str(r->header_name_start, r->header_name_end);
            // print_str(r->header_start, r->header_end);
            return res;
        }
        if (res == AGAIN) {
            fputs("AGAIN......................", stderr);
            // print_str(r->header_name_start, r->header_name_end);
            // print_str(r->header_start, r->header_end);
            return res;
        }
        // printf("res is %d\n", res);
        if (res == OK) {
            fputs(".OK.....................", stderr);
            print_str(r->header_name_start, r->header_name_end);
            print_str(r->header_start, r->header_end);
            fprintf(stderr, "\n");
            // char key[64];
            // char value[64];
            // str_to_buf_lowcase(key, r->header_name_start, r->header_name_end, sizeof(key));
            // str_to_buf_lowcase(value, r->header_start, r->header_end, sizeof(value));

            // fprintf(stderr, "key: %.*s\n", (int)strlen(key), key);
            // fprintf(stderr, "value: %.*s\n", (int)strlen(value), value);


            // if (!strcmp(key, "connection")) {
            //     if (!strcmp(value, "keep-alive"))
            //         r->keep_alive = 1;
            //     else
            //         r->keep_alive = 0;
            //     LOGE("keep-alive: %d\n", r->keep_alive);
            // }
            // else if (!strcmp(key, "content-length")) {
            //     r->content_length = atol(value);
            //     LOGE("content-length: %d\n", r->content_length);
            // }   
        }
    }
    
    return res;
}




#endif