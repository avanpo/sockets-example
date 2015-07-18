
#include <sys/socket.h>

#include "sockets-example.h"

// static function declarations
static int send_message(struct socket *, struct sockaddr *, socklen_t, char *);

// send_message sends a datagram to the specified remote address
int send_message(struct socket *sock, struct sockaddr *addr,
        socklen_t addrlen, char *message){
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
