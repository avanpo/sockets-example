
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define error_fmt(f, e) fprintf(stderr, "%s: %s() error: %s\n", program_name, \
        #f, e)
#define error(f) error_fmt(f, strerror(errno))

// type declarations/definitions
struct socket {
    int fd;
    char *buffer;
    int buf_length;
};

// function declarations
char **get_options();
struct socket *open_socket(int);
void close_socket(struct socket *);
int get_address(struct sockaddr **, socklen_t *, char *, char *);
int send_message(struct socket *, struct sockaddr *, socklen_t, char *);

// external globals declarations
extern char **environ;

// static globals definitions
char *program_name;
// options is null-terminated so get_options knows how many there are
char const * const options[] = { "host", "port", "source", "message", NULL };
enum option_index { HOST, PORT, SOURCE, MESSAGE };

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    // option_values are accessed as option_value[option_index]
    char **option_values = get_options();
    // Using a buffer length of 512 bytes to stay within the usual MTUs
    struct socket *sock = open_socket(512);
    struct sockaddr *addr;
    socklen_t addrlen;
    if (get_address(&addr, &addrlen, option_values[HOST], option_values[PORT]))
        return EXIT_FAILURE;
    if (send_message(sock, addr, addrlen, option_values[MESSAGE]))
        return EXIT_FAILURE;
    close_socket(sock);
    return EXIT_SUCCESS;
}

// get_options returns an array of option values, with the indices corresponding
// to those of options
char **get_options(){
    char **values;
    for (int i = 0;; i++)
        if (!options[i]){
            values = calloc(i, sizeof(char *));
            break;
        }
    for (char **e = environ; *e; e++)
        for (int i = 0; (*e)[i] != '\0'; i++)
            if ((*e)[i] == '='){
                for (int j = 0; options[j]; j++)
                    if (!strncmp(options[j], *e, i) && options[j][i] == '\0'){
                        values[j] = *e + i + 1;
                        break;
                    }
                break;
            }
    return values;
}

// open_socket returns a UDP socket and buffer
struct socket *open_socket(int buf_length){
    struct socket *sock = malloc(sizeof(struct socket));
    if (!sock){
        error(malloc);
        return NULL;
    }
    sock->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock->fd == -1){
        error(socket);
        free(sock);
        return NULL;
    }
    sock->buffer = calloc(1, buf_length);
    if (!sock->buffer){
        error(calloc);
        free(sock);
        return NULL;
    }
    sock->buf_length = buf_length;
    return sock;
}

void close_socket(struct socket *sock){
    if (close(sock->fd)) error(close);
    free(sock->buffer);
    free(sock);
    return;
}

// get_address takes references to *addr and addrlen and populates them
int get_address(struct sockaddr **addr, socklen_t *addrlen, char *host,
        char *port){
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    struct addrinfo *result;
    int gai_errno = getaddrinfo(host, port, &hints, &result);
    if (gai_errno == EAI_SYSTEM){
        error(getaddrinfo);
        return -1;
    }
    else if (gai_errno){
        error_fmt(getaddrinfo, gai_strerror(gai_errno));
        return -1;
    }
    *addr = result->ai_addr;
    *addrlen = result->ai_addrlen;
    freeaddrinfo(result);
    return 0;
}

int send_message(struct socket *sock, struct sockaddr *addr, socklen_t addrlen,
        char *message){
    int length = strlen(message);
    if (length >= sock->buf_length)
        length = sock->buf_length - 1;
    strncpy(sock->buffer, message, length);
    // use length + 1 to include a null terminator
    if (sendto(sock->fd, sock->buffer, length + 1, 0, addr, addrlen) == -1){
        error(sendto);
        return -1;
    }
    return 0;
}
