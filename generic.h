#ifndef GENERIC_H
#define GENERIC_H

#include "list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 1024

#define MIN(a, b) ({\
    typeof(a) _a = a; \
    typeof(b) _b = b;  \
    _a < _b?_a:_b;})

#define MAX(a, b) ({\
    typeof(a) _a = a; \
    typeof(b) _b = b;  \
    _a > _b?_a:_b;})

#define LF (u_char)'\n'
#define CR (u_char)'\r'
#define CRLF "\r\n"

#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

// static char *header_name_str[] = {"Content-Length", "Connection", "Content-Type", "Date"};

#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"
#define CONNECTION "Connection"

typedef unsigned char u_char;
typedef unsigned int uint_t;
typedef int int_t;
typedef unsigned int uint32_t;


// typedef struct http_request http_request_t;

typedef struct http_buf http_buf_t;


typedef struct {
    size_t len;
    u_char *data;
} str_t;

/* parse request line */

#define HTTP_UNKNOWN                   0x00000001
#define HTTP_GET                       0x00000002
#define HTTP_POST                      0x00000008

#define PARSE_BEGIN 0
#define PARSE_HEADER 1
#define PARSE_HEADER_DONE 2
#define PARSE_BODY 3

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

#define RESPONSE_IN 0
#define RESPONSE_END 1

#define WRITE_BEGIN 0
#define WRITE_HEADER 1
#define WRITE_BODY 2

#define SESSION_INIT 0 
#define SESSION_READ 1
#define SESSION_END 2

#define FREE_NONE 0
#define FREE_MALLOC 1
#define FREE_MMAP 2

/* respond */
#define RSP_OK                          "200 OK"
#define RSP_BAD_REQUEST                 "400 Bad Request"
#define RSP_NOT_FOUND                   "404 Not found"
#define RSP_METHOD_NOT_ALLOWED          "405 Method Not Allowed"
#define RSP_REQUEST_TIMEOUT             "408 Request Timeout"
#define RSP_LENGTH_REQUIRED             "411 Length Required"
#define RSP_PAYLOAD_TOO_LARGE           "413 Payload Too Large"
#define RSP_URI_TOO_LONG                "414 URI Too Long"
#define RSP_REQUEST_HEADER_FIELDS_TOO_LARGE "431 Request Header Fields Too Large"
#define RSP_INTERNAL_SERVER_ERROR       "500 Internal Server Error"
#define RSP_HTTP_VERSION_NOT_SUPPORTED  "505 HTTP Version Not Supported"


static void error(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
};

typedef struct http_header_s {
    char const *key;
    char const *value;
    struct http_header_s *next;
} http_header_t;

typedef struct http_response_s {
    char *status;     /* the status code for response */
    int pos;        /* the pos int buffer */
    int work_state; /* write header or write body */
    http_header_t *headers;
    int header_length;
    char *body;
    int body_length;
    int body_memop;      /* whether need to free the body */
    struct http_response_s *next;
} http_response_t;


typedef struct http_request_s {
    int fd;
    int epfd;
    u_char buf[BUFSIZE];    /* usef for request */
    u_char *pos;
    u_char *last;


    u_char out_buf[BUFSIZE];    /* used to response header */
    http_response_t* responses; /* list of response */
    
    /* state for HTTP request line and  HTTP headers */
    uint_t                        state;

    /* state for parse HTTP CONTENT */
    uint_t parse_state;

    /* state for connection */
    unsigned                          http_state:4;

    uint_t                        header_hash;
    uint_t                        lowcase_index;
    u_char                            lowcase_header[HTTP_LC_HEADER_LEN];

    u_char                           *header_name_start;
    u_char                           *header_name_end;
    u_char                           *header_start;
    u_char                           *header_end;

    /* used to parse HTTP request line */
    u_char                           *uri_start;
    u_char                           *uri_end;
    u_char                           *uri_ext;
    u_char                           *args_start;       /* url arguments */
    u_char                           *request_start;    /* request line start */
    u_char                           *request_end;      /* request line end */
    u_char                           *method_end;       /* method end */
    u_char                           *host_start;
    u_char                           *host_end;
    u_char                           *port_start;       /* url port start */
    u_char                           *port_end;

    unsigned                          http_minor:16;
    unsigned                          http_major:16;

    uint_t                        method;
    uint_t                        http_version;

    unsigned                          count:16;
    unsigned                          subrequests:8;
    unsigned                          blocked:8;

    unsigned                          aio:1;

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

} http_request_t;

struct http_buf {
    u_char *pos;
    u_char *last;
};

extern int http_parse_request_line(http_request_t *r);
extern int_t ngx_http_parse_header_line(http_request_t *r, uint_t allow_underscores);

extern int http_parse_header_lines(http_request_t *r); 


extern int create_and_bind(const char* port);

extern void shift_buf(http_request_t *session, u_char *target);
extern void do_response_old(http_request_t *session);
extern void do_request(http_request_t *session);

extern void do_respond(http_request_t *session);

extern int atoi_hs(const char *start, const char *end);

// static char ngx_http_error_494_page[] =
// "<html>" CRLF
// "<head><title>400 Request Header Or Cookie Too Large</title></head>"
// CRLF
// "<body>" CRLF
// "<center><h1>400 Bad Request</h1></center>" CRLF
// "<center>Request Header Or Cookie Too Large</center>" CRLF
// ;



// static char *http_error_page(const char * rsp_state)
// {
//     static char buf[256];
//     int nwrite;
//     nwrite = sprintf(buf, "<html>" CRLF
//                         "<head><title>%s</title></head>" CRLF
//                         "<body>" CRLF
//                         "<center><h1>%s</h1></center>" CRLF, rsp_state, rsp_state);
//     return buf;
// }


#endif