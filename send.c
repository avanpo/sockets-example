
#include <stdlib.h>
#include <sys/socket.h>

#include "sockets-example.h"

// send_start directs datagrams from stdin to a given address
//  it expects a header of the form
//      hostname:port:length\n
//  followed by a message of the specified length
void *send_start(void *arg){
    int sockfd = *(int *) arg;
    // using a buffer length of 512 bytes to stay within the usual MTUs
    const int buf_length = 512;
    void *buffer = malloc(buf_length);
    if (!buffer){
        error(malloc);
        fatal("allocation failure");
    }
    while (1){
        int length;
        char host[51];
        char port[6];
        struct sockaddr *addr;
        socklen_t addrlen;
        int ret = scanf("%50[0-9.a-zA-Z]:%5[0-9]:%d%*1[\n]", host, port,
                &length);
        if (ret == EOF){
            if (ferror(stdin)){
                error(scanf);
                fatal("failure reading from stdin");
            }
            // On EOF we assume success
            exit(EXIT_SUCCESS);
        }
        if (ret != 3) fatal("invalid header");
        if (get_address(&addr, &addrlen, host, port)) fatal("invalid address");
        if (length > buf_length) fatal("length too large");
        ret = fread(buffer, 1, length, stdin);
        if (ret < length){
            if (ferror(stdin)){
                error(ferror);
                fatal("failure reading from stdin");
            }
            fatal("message length mismatch");
        }
        if (sendto(sockfd, buffer, length, 0, addr, addrlen) == -1){
            error(sendto);
            fatal("failure to send datagram");
        }
    }
    free(buffer);
    return EXIT_SUCCESS;
}
