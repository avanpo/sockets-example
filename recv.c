
#include <sys/socket.h>

#include "sockets-example.h"

// static function declarations
static int recv_message(struct socket *);

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

// recv_start is the start function for the reader thread
void *recv_start(void *arg){
    struct socket *sock = (struct socket *) arg;
    unsigned long sequence_no = 0;
    while (!recv_message(sock))
        printf("%s\n%ld\n", sock->buffer, sequence_no++);
    return NULL;
}
