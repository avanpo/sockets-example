#ifndef SOCKETS_EXAMPLE_H
#define SOCKETS_EXAMPLE_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

// error macro definitions
#define error_fmt(f, e) fprintf(stderr, "%s: %s() error: %s\n", program_name, \
        #f, e)
#define error_no(f, n) error_fmt(f, strerror(n))
#define error(f) error_no(f, errno)

// extern function declarations
extern void *recv_start(void *);

// extern globals declarations
extern char const *program_name;

#endif
