
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define error_fmt(f, e) fprintf(stderr, "%s: %s() error: %s\n", program_name, \
        #f, e)
#define error(f) error_fmt(f, strerror(errno))

extern char **environ;

char **get_options();
void get_address(struct sockaddr **, socklen_t *, char *, char *);
int client(char **);

char *program_name;
// options is null-terminated so get_options knows how many there are.
char const * const options[] = { "host", "port", "source", NULL };
enum option_index { HOST, PORT, SOURCE };

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    // option_values are accessed as option_value[option_index].
    char **option_values = get_options();
    return client(option_values);
}

// get_options returns an array of option values, with the indices corresponding
// to those of options.
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

void get_address(struct sockaddr **addr, socklen_t *addrlen, char *host,
        char *port){
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    struct addrinfo *result;
    int gai_errno = getaddrinfo(host, port, &hints, &result);
    if (gai_errno == EAI_SYSTEM)
        error(getaddrinfo);
    else if (gai_errno)
        error_fmt(getaddrinfo, gai_strerror(gai_errno));
    *addr = result->ai_addr;
    *addrlen = result->ai_addrlen;
    freeaddrinfo(result);
    return;
}

int client(char **o_values){
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1)
        error(socket);
    struct sockaddr *addr;
    socklen_t addrlen;
    get_address(&addr, &addrlen, o_values[HOST], o_values[PORT]);

    char *buffer = "testing transfer through UDP.\n";
    if (sendto(sockfd, buffer, sizeof(buffer), 0, addr, addrlen) == -1)
        error(sendto);

    if (close(sockfd))
        error(close);
    return EXIT_SUCCESS;
}

