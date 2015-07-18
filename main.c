
#include <libgen.h>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "sockets-example.h"

// extern function declarations
extern int get_address(struct sockaddr **, socklen_t *, char *, char *);

// static function declarations
static char **get_options();
static struct socket *open_socket(int);
static void close_socket(struct socket *);
static int bind_socket(struct socket *, char *);

// extern globals declarations
extern char **environ;

// globals definitions
char const *program_name;
// options is null-terminated so get_options knows how many there are
static char const * const options[] = { "host", "local_port", "remote_port",
    NULL };
enum option_index { HOST, LOCAL_PORT, REMOTE_PORT };

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    // option_values are accessed as option_value[option_index]
    char **option_values = get_options();
    // Using a buffer length of 512 bytes to stay within the usual MTUs
    struct socket *sock = open_socket(512);
    if (bind_socket(sock, option_values[LOCAL_PORT]))
        return EXIT_FAILURE;
    // create thread for reading incoming datagrams
    pthread_t recv_thread;
    int err = pthread_create(&recv_thread, NULL, recv_start, sock);
    if (err){
        error_no(pthread_create, err);
        return EXIT_FAILURE;
    }
    // wait for recv_thread before closing socket
    void *recv_thread_ret;
    err = pthread_join(recv_thread, &recv_thread_ret);
    if (err){
        error_no(pthread_join, err);
        return EXIT_FAILURE;
    }
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
