#ifndef GENERIC_H
#define GENERIC_H

#include <stdio.h>

typedef unsigned char u_char;
typedef unsigned int ngx_uint_t;
typedef int ngx_int_t;

typedef struct http_request http_request_t;

typedef struct http_buf ngx_buf_t;


typedef struct {
    size_t len;
    u_char *data;
} ngx_str_t;


#define LF (u_char)'\n'
#define CR (u_char)'\r'
#define CRLF "\r\n"

#define NGX_HTTP_UNKNOWN                   0x00000001
#define NGX_HTTP_GET                       0x00000002
#define NGX_HTTP_HEAD                      0x00000004
#define NGX_HTTP_POST                      0x00000008

// GET /rfc/rfc2616.txt HTTP/2

#define NGX_HTTP_PARSE_HEADER_DONE         1

#define NGX_HTTP_CLIENT_ERROR              10
#define NGX_HTTP_PARSE_INVALID_METHOD      10
#define NGX_HTTP_PARSE_INVALID_REQUEST     11
#define NGX_HTTP_PARSE_INVALID_VERSION     12
#define NGX_HTTP_PARSE_INVALID_09_METHOD   13

#define NGX_HTTP_PARSE_INVALID_HEADER      14

#define NGX_OK 0
#define NGX_ERROR -1
#define NGX_AGAIN -2

#define NGX_HTTP_LC_HEADER_LEN 32


struct http_request {
    int fd;
    
     /* used to parse HTTP headers */
    ngx_uint_t                        state;

    ngx_uint_t                        header_hash;
    ngx_uint_t                        lowcase_index;
    u_char                            lowcase_header[NGX_HTTP_LC_HEADER_LEN];

    u_char                           *header_name_start;
    u_char                           *header_name_end;
    u_char                           *header_start;
    u_char                           *header_end;

    /*
     * a memory that can be reused after parsing a request line
     * via ngx_http_ephemeral_t
     */

    u_char                           *uri_start;
    u_char                           *uri_end;
    u_char                           *uri_ext;
    u_char                           *args_start;
    u_char                           *request_start;
    u_char                           *request_end;
    u_char                           *method_end;
    u_char                           *schema_start;
    u_char                           *schema_end;
    u_char                           *host_start;
    u_char                           *host_end;
    u_char                           *port_start;
    u_char                           *port_end;

    unsigned                          http_minor:16;
    unsigned                          http_major:16;
    ngx_uint_t header_hash;
    ngx_uint_t lowcase_index;

    unsigned invalid_header:1;
    u_char lowcase_header[NGX_HTTP_LC_HEADER_LEN];

    ngx_uint_t                        method;
    ngx_uint_t                        http_version;

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

    ngx_str_t                         http_protocol;
};

struct http_buf {
    u_char *pos;
    u_char *last;

};

#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

#endif