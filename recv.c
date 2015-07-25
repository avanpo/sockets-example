
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
    // NULL-terminating buffer so it can optionally be used as a string
    unsigned char buf[UDP_MAX_PAYLOAD + 1];
    while (1){
        struct datagram dg = { sockfd, buf };
        recv_message(&dg);
        char address[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &dg.src_address, address, INET_ADDRSTRLEN))
            ; //error
        int port = ntohs(dg.src_port);
        int ret = printf("%d byte%s from %s:%d", dg.length,
                dg.length != 1 ? "s" : "", address, port);
        if (ret < 0); //error
        int i = 0;
        for (; i < dg.length; i++)
            if (!isprint(buf[i])) break;
        // if any characters in buf aren't printable, print a hexadecimal
        // representation instead
        if (i < dg.length){
            // 5 characters for every 2 input
            char hex[(5 * UDP_MAX_PAYLOAD / 2) + 1];
            int p = 0;
            for (i = 0; i < dg.length; i++){
                char a = buf[i] >> 4;
                char b = buf[i] & 0x0f;
                hex[p++] = a < 10 ? a + '0' : a - 10 + 'a';
                hex[p++] = b < 10 ? b + '0' : b - 10 + 'a';
                if (!((i + 1) % 2))
                    hex[p++] = ' ';
            }
            hex[5 * dg.length / 2] = '\0';
            ret = printf(" (in hexadecimal)\n%s\n", hex);
            if (ret < 0); //error
            continue;
        }
        ret = printf("\n%s\n", buf);
        if (ret < 0); //error
    }
}

// recv_message waits for an incoming datagram and writes it to dg->buffer
void recv_message(struct datagram *dg){
    struct sockaddr_in sa;
    socklen_t salen = sizeof(struct sockaddr_in);
    int ret = recvfrom(dg->sockfd, dg->buffer, UDP_MAX_PAYLOAD, MSG_TRUNC,
            (struct sockaddr *) &sa, &salen);
    if (ret == -1); //error
    dg->src_address = sa.sin_addr.s_addr;
    dg->src_port = sa.sin_port;
    dg->too_large = ret < UDP_MAX_PAYLOAD ? false : true;
    dg->length = dg->too_large ? UDP_MAX_PAYLOAD : ret;
    dg->buffer[dg->length] = '\0';
}
