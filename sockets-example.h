#ifndef SOCKETS_EXAMPLE_H
#define SOCKETS_EXAMPLE_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define error_fmt(f, e) fprintf(stderr, "%s: %s() error: %s\n", program_name, \
        #f, e)
#define error(f) error_fmt(f, strerror(errno))

// type definitions
struct socket {
    int fd;
    char *buffer;
    int buf_length;
};

// external globals declarations
extern char **environ;
extern char const *program_name;

#endif
