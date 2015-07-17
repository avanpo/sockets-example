
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;
extern char *program_name;

char **get_options(char **);
int client(char *, char *, char *);
int server(char *, char *);

int main(int argc, char **argv){
    char *program_name = basename(argv[0]);
    // options is null-terminated so get_options knows how many there are.
    char *options[] = { "role", "host", "port", "source", NULL };
    enum option_index { ROLE, HOST, PORT, SOURCE };
    char **option_values = get_options(options);
/*    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1)
        fprintf(stderr, "%s: socket() error: %s\n", program_name, strerror(errno));
    int port = atoi(options[2].value);
    int address;
    struct addrinfo *res;
    if (getaddrinfo(options[1].value, options[2].value, NULL, &res))
        fprintf(stderr, "%s: getaddrinfo() error: %s\n", program_name, strerror(errno));
    const struct sockaddr saddr = {
        AF_INET, htons(port), htons(address)
    };
*/    if (!strcmp(options[ROLE], "client"))
        return client(options[HOST], options[PORT], options[SOURCE]);
    else if (!strcmp(options[ROLE], "server"))
        return server(options[HOST], options[PORT]);
}

// get_options returns an array of option values, with the indices corresponding
// to those of options.
char **get_options(char **options){
    char **values;
    for (int i = 0;; i++)
        if (!options[i]){
            values = calloc(i, sizeof(char *));
            break;
        }
    for (char **e = environ; *e; e++){
        char *env = *e;
        for (int i = 0; env[i] != '\0'; i++){
            if (env[i] == '='){
                for (int j = 0; options[j]; j++){
                    if (!strncmp(options[j], env, i) && options[j][i] == '\0'){
                        values[j] = env + i + 1;
                        break;
                    }
                }
                break;
            }
        }
    }
    return values;
}

int client(char *host, char *port, char *source){
/*    if (connect(sd, &saddr, sizeof(saddr))){
        fprintf(stderr, "%s: connect() error: %s\n", program_name,
                strerror(errno));
        close(sd);
    }*/
    return 0;
}

int server(char *host, char *port){
    return 0;
}
