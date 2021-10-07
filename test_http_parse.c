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

#include "generic.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void print_str(char *start, char *end)
{
    char *p;
    for (p = start; p != end; ++p)
        putc(*p, stdout);

    puts("");
}

int main(int argc, char *argv[])
{
    char buf[BUFSIZ];
    int nread;
    unsigned int offset;


    while ((nread = read(STDIN_FILENO, buf + offset, BUFSIZ - offset)) > 0) 
        offset += nread;
    
    ngx_buf_t nb;
    nb.pos = buf;
    nb.last = buf + offset;

    http_request_t req;
    http_parse_request_line(&req, &nb);

    printf("method: ");
    print_str(req.request_start, req.method_end + 1);

    printf("uri: ");
    print_str(req.uri_start, req.uri_end + 1);
    
    printf("http_version: major %d, minor %d, version %d\n", req.http_major, req.http_minor, req.http_version);

    return 0;
}