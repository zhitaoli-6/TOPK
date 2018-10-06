#ifndef COMMON_H
#define COMMON_H


#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>


typedef unsigned long long ull;

#define MIN_FILE_SIZE (1<<30)

#define MAX_URL_LEN (8)
#define MIN_URL_LEN (4)

#define SHARD_SIZE (128)



#define TIME(a,b) (1.0*((b).tv_sec-(a).tv_sec)+0.000001*((b).tv_usec-(a).tv_usec))

#define EXIT_IF_NULL(ptr) \
    do { \
        if (ptr == NULL) { \
            fprintf(stderr, "FATAL: INVALID Pointer. (%s: Line %d)\n", \
                    __FILE__, __LINE__); \
            exit(1); \
        }\
    } while(0)

#define EXIT(msg) \
    do { \
        fprintf(stderr, "FATAL: %s (%s: Line %d)\n", \
                msg, __FILE__, __LINE__); \
        exit(1); \
    } while(0)
    
#endif
