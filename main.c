
#include <arpa/inet.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "sockets-example.h"

/// static function declarations
static in_port_t set_port();
static int open_socket(uint16_t);

/// globals definitions
char const *program_name;

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    int sockfd = open_socket(set_port());
    // create thread for sending datagrams
    pthread_t send_thread;
    pthread_create(&send_thread, NULL, send_start, &sockfd);
    recv_start(sockfd);
    // wait for send_thread before closing socket
    pthread_join(send_thread, NULL);
    close(sockfd);
    return 0;
}

// set_port returns the port number in network byte order as decided by the user
in_port_t set_port(){
    printf("UDP listening port (0 for ephemeral): ");
    int port;
    while (1){
        int ret = scanf("%d", &port);
        if (ret == 1){
            if (port > 0xffff){
                printf("Port number too large, try again: ");
                continue;
            }
            if (port && port < 1024 && geteuid() != 0){
                printf("Need to be root to bind to port %d, try again: ", port);
                continue;
            }
            return (in_port_t) htons(port);
        }
        if (ret == EOF){
            printf("%d\n", ret);
            // user closed stream
            if (feof(stdin)){
                printf("\n");
                exit(0);
            }
            if (ferror(stdin)) printf("Stream error (stdin).\n");
        }
        printf("Invalid input, try again: ");
    }
    return 0;
}

// open_socket returns a UDP socket, bound to the specified port
//  passing 0 to port lets the kernel choose an ephemeral port
int open_socket(in_port_t port){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) eprint(errno);
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    sa.sin_family = AF_INET;
    sa.sin_port = port;
    sa.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, (struct sockaddr *) &sa, salen);
    getsockname(sockfd, (struct sockaddr *) &sa, &salen);
    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sa.sin_addr, address, INET_ADDRSTRLEN);
    int bound_port = ntohs(sa.sin_port);
    printf("Listening for datagrams on %s:%d\n", address, bound_port);
    return sockfd;
}

