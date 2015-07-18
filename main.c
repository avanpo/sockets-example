
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
int client(char **);
int server(char **);

char *program_name;
// options is null-terminated so get_options knows how many there are.
char const * const options[] = { "role", "host", "port", "source", NULL };
enum option_index { ROLE, HOST, PORT, SOURCE };

int main(int argc, char **argv){
    program_name = basename(argv[0]);
    // option_values are accessed as option_value[option_index].
    char **option_values = get_options();
    if (!strcmp(option_values[ROLE], "client"))
        return client(option_values);
    else if (!strcmp(option_values[ROLE], "server"))
        return server(option_values);
    else
        return EXIT_FAILURE;
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

int client(char **o_values){
/*    if (connect(sd, &saddr, sizeof(saddr))){
        fprintf(stderr, "%s: connect() error: %s\n", program_name,
                strerror(errno));
        close(sd);
    }*/
/*    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1)
        fprintf(stderr, "%s: socket() error: %s\n", program_name, strerror(errno));
    int port = atoi(options[2].value);
    int address;
    struct sockaddr const saddr = {
        AF_INET, htons(port), htons(address)
    };
*/
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_CANONNAME;
    struct addrinfo *result;
    int gai_errno = getaddrinfo(o_values[HOST], o_values[PORT], &hints,
            &result);
    if (gai_errno == EAI_SYSTEM)
        error(getaddrinfo);
    else if (gai_errno)
        error_fmt(getaddrinfo, gai_strerror(gai_errno));
    int sockfd;
    for (struct addrinfo *cur = result; cur; cur = cur->ai_next){
        sockfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (sockfd == -1){
            if (!cur->ai_next)
                error(socket);
            continue;
        }
    }
    freeaddrinfo(result);




    if (close(sockfd))
        error(close);
    return EXIT_SUCCESS;
}

int server(char **o_values){
    return EXIT_SUCCESS;
}
