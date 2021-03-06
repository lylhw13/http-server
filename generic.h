#ifndef GENERIC_H
#define GENERIC_H

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

#define LF (char)'\n'
#define CR (char)'\r'
#define CRLF "\r\n"

/* support method of HTTP */
#define HTTP_UNKNOWN                   0x00000001
#define HTTP_GET                       0x00000002
#define HTTP_POST                      0x00000004

/* request parse state */
#define PARSE_BEGIN 0
#define PARSE_HEADER 1
#define PARSE_HEADER_DONE 2
#define PARSE_BODY 3

/* response write state */
#define WRITE_BEGIN 0
#define WRITE_HEADER 1
#define WRITE_BODY 2

/* connection state */
#define SESSION_INIT 0 
#define SESSION_READ 1
#define SESSION_END 2

/* whether need to free body */
#define FREE_NONE 0
#define FREE_MALLOC 1
#define FREE_MMAP 2

/* state for parse */
#define OK 0
#define AGAIN 1

/* parse request line */
#define HTTP_PARSE_INVALID_METHOD      -1
#define HTTP_PARSE_INVALID_REQUEST     -2
#define HTTP_PARSE_INVALID_VERSION     -3
#define HTTP_PARSE_INVALID_09_METHOD   -4

/* parse headers */
#define HTTP_PARSE_HEADER_DONE         2
#define HTTP_PARSE_INVALID_HEADER      -1

#define HTTP_LC_HEADER_LEN 32

/* respond */
#define RSP_OK                              "200 OK"
#define RSP_BAD_REQUEST                     "400 Bad Request"
#define RSP_NOT_FOUND                       "404 Not found"
#define RSP_METHOD_NOT_ALLOWED              "405 Method Not Allowed"
#define RSP_REQUEST_TIMEOUT                 "408 Request Timeout"
#define RSP_LENGTH_REQUIRED                 "411 Length Required"
#define RSP_PAYLOAD_TOO_LARGE               "413 Payload Too Large"
#define RSP_URI_TOO_LONG                    "414 URI Too Long"
#define RSP_REQUEST_HEADER_FIELDS_TOO_LARGE "431 Request Header Fields Too Large"
#define RSP_INTERNAL_SERVER_ERROR           "500 Internal Server Error"
#define RSP_HTTP_VERSION_NOT_SUPPORTED      "505 HTTP Version Not Supported"

#define CONTENT_LENGTH "Content-Length"
#define CONTENT_TYPE "Content-Type"
#define CONNECTION "Connection"

#ifdef DEBUG
    #define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#else
    #define LOGD(...) 
#endif

#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))


typedef unsigned int uint_t;
typedef unsigned int uint32_t;

typedef struct {
    size_t len;
    char *data;
} str_t;

typedef struct http_header_s {
    char const *key;
    char const *value;
    struct http_header_s *next;
} http_header_t;

typedef struct http_response_s {
    char *status;                   /* the status code for response */
    int pos;                        /* the pos int buffer */
    int work_state;                 /* write header or write body */
    http_header_t *headers;
    int header_length;
    char *body;
    int body_length;
    int body_memop;                 /* whether need to free the body */
    struct http_response_s *next;
} http_response_t;


typedef struct http_request_s {
    int fd;
    int epfd;

    /* buf for read request */
    char buf[BUFSIZE];   
    char *pos;
    char *last; /* not include */


    char out_buf[BUFSIZE];    /* used to response header */
    http_response_t* responses; /* list of response */
    
    /* state for HTTP request line and  HTTP headers */
    uint_t                        state;

    /* state for parse HTTP CONTENT */
    uint_t parse_state;

    /* state for connection */
    unsigned                          http_state:4;

    uint_t                        header_hash;
    uint_t                        lowcase_index;
    char                            lowcase_header[HTTP_LC_HEADER_LEN];

    char                           *header_name_start;
    char                           *header_name_end;
    char                           *header_start;
    char                           *header_end;

    /* used to parse HTTP request line */
    char                           *uri_start;
    char                           *uri_end;
    char                           *uri_ext;
    char                           *args_start;       /* url arguments */
    char                           *request_start;    /* request line start */
    char                           *request_end;      /* request line end */
    char                           *method_end;       /* method end */
    char                           *host_start;
    char                           *host_end;
    char                           *port_start;       /* url port start */
    char                           *port_end;

    unsigned                          http_minor:16;
    unsigned                          http_major:16;

    uint_t                        method;
    uint_t                        http_version;

    unsigned                          count:16;
    unsigned                          subrequests:8;
    unsigned                          blocked:8;

    unsigned keep_alive:1;
    uint_t content_length;


    /* URI with "%" */
    unsigned                          quoted_uri:1;

    /* URI with empty path */
    unsigned                          empty_path_in_uri:1;

    unsigned                          invalid_header:1;

    str_t                         http_protocol;

} http_request_t;

/* http parse */
extern int http_parse_request_line(http_request_t *r);
extern int http_parse_headers(http_request_t *r); 

/* http */
extern int create_and_bind(const char* port);
extern void do_request(http_request_t *session);
extern void do_respond(http_request_t *session);

/* helper */
extern int atoi_hs(const char *start, const char *end);
extern void error(const char *str);


#endif