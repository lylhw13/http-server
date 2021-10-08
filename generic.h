#ifndef GENERIC_H
#define GENERIC_H

#include <stdio.h>

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

#define HTTP_UNKNOWN                   0x00000001
#define HTTP_GET                       0x00000002
#define HTTP_POST                      0x00000008

// GET /rfc/rfc2616.txt HTTP/2

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


struct http_request {
    int fd;
    u_char buf[BUFSIZ];
    u_char *pos;
    u_char *last;
    
     /* used to parse HTTP headers */
    uint_t                        state;

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

extern int http_parse_request_line(http_request_t *r, http_buf_t *b);
extern int_t ngx_http_parse_header_line(http_request_t *r, http_buf_t *b, uint_t allow_underscores);


#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

#endif