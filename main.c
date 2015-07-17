
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct option {
    char *name;
    char *value;
};

extern char **environ;
extern char *program_name;

int main(int argc, char **argv){
    char *program_name = basename(argv[0]);
    struct option options[] = {
        {"role"},
        {"host"},
        {"port"},
        {"source"}
    };
    for (char **e = environ; *e; e++){
        char *env = *e;
        for (int i = 0; env[i] != '\0'; i++){
            if (env[i] == '='){
                for (int j = 0; j < sizeof(options)/sizeof(struct option); j++){
                    if (!strncmp(options[j].name, env, i)
                            && options[j].name[i] == '\0'){
                        options[j].value = env + i + 1;
                        break;
                    }
                }
                break;
            }
        }
    }
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
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
    if (!strcmp(options[0].value, "client")){
        if (connect(sd, &saddr, sizeof(saddr))){
            fprintf(stderr, "%s: connect() error: %s\n", program_name,
                    strerror(errno));
            close(sd);
        }
    }
    else if (!strcmp(options[0].value, "server")){
    }
    return 0;
}
