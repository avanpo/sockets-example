
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct option {
    char *name;
    char *value;
};

extern char **environ;

int main(int argc, char **argv){
    struct option options[] = {
        {"role"},
        {"host"},
        {"port"}
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
    return 0;
}
