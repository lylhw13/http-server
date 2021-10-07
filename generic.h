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
    unsigned int state;

    unsigned char *header_name_start;
    unsigned char *header_name_end;
    unsigned char *header_start;
    unsigned char *header_end;

    u_char *request_start;
    u_char *request_end;
    u_char *method_end;
    u_char *method;
    u_char *uri_start;
    u_char *uri_end;
    u_char *schema_start;
    u_char *schema_end;
    u_char *host_start;
    u_char *host_end;
    u_char *args_start;
    u_char *empty_path_in_uri;
    u_char *port_start;
    u_char *port_end;
    u_char *complex_uri;
    u_char *quoted_uri;
    u_char *plus_in_uri;
    u_char *uri_ext;

    ngx_str_t                         request_line;
    ngx_str_t                         uri;
    ngx_str_t                         args;
    ngx_str_t                         exten;
    ngx_str_t                         unparsed_uri;

    ngx_str_t                         method_name;
    ngx_str_t                         http_protocol;
    ngx_str_t                         schema;

    ngx_uint_t method;
    ngx_uint_t http_version;

    unsigned http_minor:16;
    unsigned http_major:16;

    ngx_uint_t header_hash;
    ngx_uint_t lowcase_index;

    unsigned invalid_header:1;
    u_char lowcase_header[NGX_HTTP_LC_HEADER_LEN];

};

struct http_buf {
    u_char *pos;
    u_char *last;

};

#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

#endif