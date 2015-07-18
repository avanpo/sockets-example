
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
int bind_socket(struct socket *, char *);
int get_address(struct sockaddr **, socklen_t *, char *, char *);
int send_message(struct socket *, struct sockaddr *, socklen_t, char *);
int recv_message(struct socket *);

// external globals declarations
extern char **environ;

// static globals definitions
char *program_name;
// options is null-terminated so get_options knows how many there are
char const * const options[] = { "host", "remote_port", "local_port", NULL };
enum option_index { HOST, REMOTE_PORT, LOCAL_PORT };

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    // option_values are accessed as option_value[option_index]
    char **option_values = get_options();
    // Using a buffer length of 512 bytes to stay within the usual MTUs
    struct socket *sock = open_socket(512);
    if (bind_socket(sock, option_values[LOCAL_PORT]))
        return EXIT_FAILURE;
    if (recv_message(sock))
        return EXIT_FAILURE;
    printf("%s", sock->buffer);
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

// close_socket closes the passed socket and frees associated memory
void close_socket(struct socket *sock){
    if (close(sock->fd)) error(close);
    free(sock->buffer);
    free(sock);
    return;
}

// bind_socket binds the socket to the specified local port if possible
//  port can be a number or a service name from /etc/services
//  passing NULL to port lets the kernel choose an ephemeral port to bind to
int bind_socket(struct socket *sock, char *port){
    struct sockaddr *addr;
    socklen_t addrlen;
    if (get_address(&addr, &addrlen, "localhost", port))
        return -1;
    if (bind(sock->fd, addr, addrlen)){
        error(bind);
        return -1;
    }
    return 0;
}

// get_address takes references to *addr and addrlen and populates them
//  host can be a hostname, or an ip address
//  port can be a number or a service name from /etc/services
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

// send_message sends a datagram to the specified remote address
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

// recv_message waits for an incoming datagram and writes it to sock->buffer
//  if the socket is unbound, this call will cause the program to hang
int recv_message(struct socket *sock){
    int ret = recv(sock->fd, sock->buffer, sock->buf_length, MSG_TRUNC);
    if (ret == -1){
        error(recv);
        return -1;
    }
    // ensure the buffer is null-terminated
    if (ret < sock->buf_length)
        sock->buffer[ret] = '\0';
    else
        sock->buffer[sock->buf_length - 1] = '\0';
    return 0;
}
