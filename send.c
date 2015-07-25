
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "sockets-example.h"

/// static function declarations
static uint32_t set_address();
static in_port_t set_port();

// send_start directs datagrams from stdin to an address entered by the user
void *send_start(void *arg){
    int sockfd = *(int *) arg;
    unsigned char buf[UDP_MAX_PAYLOAD];
    while (1){
        struct sockaddr_in sa;
        socklen_t salen = sizeof(sa);
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = set_address();
        sa.sin_port = set_port();
        int ret, count = 0;
        // flush newline from last input
        fgetc(stdin);
        while ((ret = fread(buf, 1, UDP_MAX_PAYLOAD, stdin))){
            sendto(sockfd, buf, ret, 0, (struct sockaddr *) &sa, salen);
            count++;
            if (ret < UDP_MAX_PAYLOAD) break;
        }
        printf("\n%d datagram%s sent.\n", count, count != 1 ? "s" : "");
    }
    return NULL;
}

// set_address returns the ipv4 address in network byte order as decided by the
// user
uint32_t set_address(){
    printf("Send to (IPv4 address): ");
    while (1){
        char a[16];
        uint32_t addr;
        int ret = scanf("%15s", a);
        if (ret == 1){
            ret = inet_pton(AF_INET, a, &addr);
            if (!ret){
                printf("Invalid input, try again: ");
                continue;
            }
            return addr;
        }
        if (ret == EOF){
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

// set_port returns the port number in network byte order as decided by the user
in_port_t set_port(){
    printf("UDP port number: ");
    int port;
    while (1){
        int ret = scanf("%d", &port);
        if (ret == 1){
            if (port > 0xffff){
                printf("Port number too large, try again: ");
                continue;
            }
            return (in_port_t) htons(port);
        }
        if (ret == EOF){
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
