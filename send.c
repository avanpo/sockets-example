
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
        while ((ret = fread(buf, 1, UDP_MAX_PAYLOAD, stdin))){
            if (sendto(sockfd, buf, ret, 0, (struct sockaddr *) &sa, salen)
                    == -1); //error
            count++;
            if (ret < UDP_MAX_PAYLOAD) break;
        }
        ret = printf("%d datagram%s sent.\n", count, count > 1 ? "s" : "");
        if (ret < 0); //error
    }
    return NULL;
}

// set_address returns the ipv4 address in network byte order as decided by the
// user
uint32_t set_address(){
    int ret = printf("Send to (IPv4 address): ");
    if (ret < 0); //error
    while (1){
        char a[16];
        uint32_t addr;
        ret = scanf("%15s", a);
        if (ret == 1){
            ret = inet_pton(AF_INET, a, &addr);
            if (!ret){
                ret = printf("Invalid input, try again: ");
                if (ret < 0); //error
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
            if (ferror(stdin)); //error
        }
        ret = printf("Invalid input, try again: ");
        if (ret < 0); //error
        // flush stdin
        while (fgetc(stdin) != EOF);
    }
    return 0;
}

// set_port returns the port number in network byte order as decided by the user
in_port_t set_port(){
    int ret = printf("UDP port number: ");
    if (ret < 0); //error
    int port;
    while (1){
        ret = scanf("%d", &port);
        if (ret == 1){
            if (port > 0xffff){
                ret = printf("Port number too large, try again: ");
                if (ret < 0); //error
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
            if (ferror(stdin)); //error
        }
        ret = printf("Invalid input, try again: ");
        if (ret < 0); //error
        // flush stdin
        while (fgetc(stdin) != EOF);
    }
    return 0;
}
