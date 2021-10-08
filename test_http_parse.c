// GET /index.html HTTP/1.1
// Host: 127.0.0.1:33333
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:93.0) Gecko/20100101 Firefox/93.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1

// POST / HTTP/1.1
// key: value
// Content-Type: text/plain
// User-Agent: PostmanRuntime/7.26.8
// Accept: */*
// Cache-Control: no-cache
// Postman-Token: 37fda8c3-206d-4d2c-b735-b808af442205
// Host: 127.0.0.1:33333
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Content-Length: 15

// this is content

#include "generic.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void print_str(char *start, char *end)
{
    char *p;
    for (p = start; p != end; ++p)
        putc(*p, stdout);

    puts("");
}


int http_parse_header_lines(http_request_t *r, http_buf_t *b) 
{
    int_t  res;
    int i;

    for (;;) {
        res = ngx_http_parse_header_line(r, b, 1);
        if (res == HTTP_PARSE_INVALID_HEADER) {
            printf("invalid");
            return -1;
        }
        if (res == HTTP_PARSE_HEADER_DONE) {
            puts("DONE......................");
            // print_str(r->header_name_start, r->header_name_end);
            // print_str(r->header_start, r->header_end);
            return 0;
        }
        if (res == AGAIN) {
            puts("AGAIN......................");
            // print_str(r->header_name_start, r->header_name_end);
            // print_str(r->header_start, r->header_end);
            break;
        }
        // printf("res is %d\n", res);
        if (res == OK) {
            puts(".OK.....................");
            print_str(r->header_name_start, r->header_name_end);
            print_str(r->header_start, r->header_end);
        }
    }
    
    return 0;
}

void wrte_body(http_request_t *r, http_buf_t *b)
{
    printf("print_body");
    print_str(b->pos, b->last);
}

/* not empty, start from /, don't contain .. */
int check_url(http_request_t *r, http_buf_t *b)
{
    if (r->uri_start == r->uri_end || *(r->uri_start) != '/')
        return -1;

    u_char *p;
    for (p = r->uri_start; (p+1) != r->uri_end; p++) {
        if (*p == '.' && *(p+1) == '.')
            return -1;
    }
    return 0;
}

void test(http_buf_t *b)
{
    static u_char a = 'c';
    b->pos = &a;
}

int main(int argc, char *argv[])
{

    // char a = 't';
    // ngx_buf_t nb;
    // nb.pos = &a;
    // printf("%c\n", *(nb.pos));

    // test(&nb);
    // printf("%c\n", *(nb.pos));
    char buf[BUFSIZ];
    int nread;
    unsigned int offset;


    while ((nread = read(STDIN_FILENO, buf + offset, BUFSIZ - offset)) > 0) 
        offset += nread;
    
    http_buf_t nb;
    nb.pos = buf;
    nb.last = buf + offset;


    http_request_t req;
    // printf("1 %c\n", *(nb.pos));


    // printf("1 nb %ld, uri %ld \n", (unsigned long)nb.pos, (unsigned long) req.uri_start);
    // printf("1 nb %c, uri %c \n", *(nb.pos), *(req.uri_start));

    // http_parse_request_line(&req, &nb);
    // printf("2 %c\n", *(nb.pos));

    // printf("2 nb %ld, uri %ld \n", (unsigned long)nb.pos, (unsigned long) req.uri_start);
    // printf("2 nb %c, uri %c \n", *(nb.pos), *(req.uri_start));



    // printf("2 %ld\n", (unsigned long)nb.pos);


    printf("method: ");
    print_str(req.request_start, req.method_end + 1);
    // print_str(req.request_start, req.request_end);  // the request line

    printf("uri: ");
    print_str(req.uri_start, req.uri_end + 1);
    
    printf("http_version: major %d, minor %d, version %d\n", req.http_major, req.http_minor, req.http_version);

    http_parse_header_lines(&req, &nb);
    // printf("3 %c\n", *(nb.pos));
    // printf("3 nb %ld, uri %ld \n", (unsigned long)nb.pos, (unsigned long) req.uri_start);
    // printf("3 nb %c, uri %c \n", *(nb.pos), *(req.uri_start));



    // return 0;
}