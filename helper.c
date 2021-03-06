#include "generic.h"
#include <limits.h>
#include <errno.h>

int atoi_hs(const char *start, const char *end)
{
    int val, width;
    char *endptr;
    char str[64];
    
    width = (int)(end - start);
    if (width <= 0 || width >= 64)   
        return -1;

    sprintf(str, "%.*s", width, start);

    errno = 0;    
    val = (int)strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
            || (errno != 0 && val == 0)) {
        perror("strtol");
        return -1;
    }

    if (endptr == str) {
        LOGE("No digits were found\n");
        return -1;
    }

    if (*endptr != '\0') {
        LOGE("Further characters after number: %s\n", endptr);
        return -1;
    }       

    return val;
}

void * xmalloc(size_t len)
{
    void *res;
    res = malloc(len);
    if (!res) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return res;
}

void error(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
};