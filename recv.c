
#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "sockets-example.h"

/// type definitions
struct datagram {
    int sockfd;
    unsigned char *buffer;
    // src_address and src_port are in network order
    uint32_t src_address;
    uint16_t src_port;
    int length;
    bool too_large;
};

/// static function declarations
static void recv_message(struct datagram *);

// recv_start begins the read loop for incoming datagrams
void recv_start(int sockfd){
    unsigned char buf[UDP_MAX_PAYLOAD + 1];
    // NULL-terminate buffer so it can optionally be used as a string
    buf[UDP_MAX_PAYLOAD] = '\0';
    while (1){
        struct datagram dg = { sockfd, buf };
        recv_message(&dg);
        char address[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &dg.src_address, address, INET_ADDRSTRLEN))
            ; //error
        int port = ntohs(dg.src_port);
        int ret = printf("From %s:%d\n", address, port);
        if (ret < 0); //error
        for (int i = 0; i < dg.length; i++){
            // if any characters in buf aren't printable, print a hexadecimal
            // representation instead
            if (!isprint(buf[i])){
                unsigned char hex[2 * UDP_MAX_PAYLOAD + 1];
                hex[2 * UDP_MAX_PAYLOAD] = '\0';
                for (i = 0; i < 2 * UDP_MAX_PAYLOAD; i++){
                    int n = i % 2 ? buf[i / 2] & 0x0f : buf[i / 2] >> 4;
                    hex[i] = n < 10 ? n + '0' : n + 'a';
                }
                ret = printf("%s\n", hex);
                if (ret < 0); //error
                break;
            }
            ret = printf("%s\n", buf);
            if (ret < 0); //error
        }
    }
}

// recv_message waits for an incoming datagram and writes it to dg->buffer
void recv_message(struct datagram *dg){
    struct sockaddr_in sa;
    socklen_t salen = sizeof(struct sockaddr_in);
    int ret = recvfrom(dg->sockfd, dg->buffer, UDP_MAX_PAYLOAD, MSG_TRUNC,
            (struct sockaddr *) &sa, &salen);
    if (ret == -1) //error
    dg->src_address = sa.sin_addr.s_addr;
    dg->src_port = sa.sin_port;
    dg->too_large = ret < UDP_MAX_PAYLOAD ? false : true;
    dg->length = dg->too_large ? UDP_MAX_PAYLOAD : ret;
}
