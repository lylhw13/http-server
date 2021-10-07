#ifndef GENERIC_H
#define GENERIC_H

#include <stdio.h>

#define LOGD(...) ((void)fprintf(stdout, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

#endif