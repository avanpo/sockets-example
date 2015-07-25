#ifndef SOCKETS_EXAMPLE_H
#define SOCKETS_EXAMPLE_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

/// constant definitions
// using a payload size of 512 bytes to stay within the usual MTUs
#define UDP_MAX_PAYLOAD 512

/// error macro definitions
#define ERROR_RETURN_ERRNO(f, ef, ...) \
    do { int err_ = f(__VA_ARGS__); if (err_){ ef(err_); exit(-1); } } while (0)
#define ERROR_NONNULL_SET_ERRNO(f, ef, ...) \
    do { if (f(__VA_ARGS__)){ ef(errno); exit(-1); } } while (0)
#define ERROR_NULL_SET_ERRNO(f, ef, ...) \
    do { if (!f(__VA_ARGS__)){ ef(errno); exit(-1); } } while (0)
#define ERROR_NEGATIVE_SET_ERRNO(f, ef, ...) \
    do { if (f(__VA_ARGS__) < 0){ ef(errno); exit(-1); } } while (0)
#define PRINT_ERROR_FMT(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define PRINT_ERROR_STR(e) \
    PRINT_ERROR_FMT("\n%s:%d: error in %s(): %s\n", __FILE__, __LINE__,\
            __func__, e)
#define PRINT_ERRNO(e) \
    do { char err_string_[64]; \
        if (strerror_r(e, err_string_, 64)) \
            PRINT_ERROR_STR("unknown"); \
        else PRINT_ERROR_STR(err_string_); \
    } while (0)
#define eprint(e) do { PRINT_ERRNO(e); exit(-1); } while (0)
#define bind(...) \
    ERROR_NONNULL_SET_ERRNO(bind, eprint, __VA_ARGS__)
#define close(fd) \
    ERROR_NONNULL_SET_ERRNO(close, eprint, fd)
#define getsockname(...) \
    ERROR_NONNULL_SET_ERRNO(getsockname, eprint, __VA_ARGS__)
#define inet_ntop(...) \
    ERROR_NULL_SET_ERRNO(inet_ntop, eprint, __VA_ARGS__)
#define pthread_create(...) \
    ERROR_RETURN_ERRNO(pthread_create, eprint, __VA_ARGS__)
#define pthread_join(...) \
    ERROR_RETURN_ERRNO(pthread_join, eprint, __VA_ARGS__)
#define sendto(...) \
    ERROR_NEGATIVE_SET_ERRNO(sendto, eprint, __VA_ARGS__)

/// extern function declarations
extern void recv_start(int);
extern void *send_start(void *);

/// extern globals declarations
extern char const *program_name;

#endif
