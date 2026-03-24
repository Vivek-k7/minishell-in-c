#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"

int parse_input(char *input, char ***cmds){

    int i = 0;
    char *raw_cmds[64];

    char* token = strtok(input, "|");
    while (token != NULL) {
        raw_cmds[i] = token;
        token = strtok(NULL, "|");
        i++;
    }
    raw_cmds[i] = NULL;
    int count = i;
    if(raw_cmds[0] == NULL) return 0;
    for(int j = 0; raw_cmds[j]!=NULL; j++){
        char **args = malloc(sizeof(char*) * 64);
        if(!args){
            perror("malloc");
            return -2;
        }
        token = strtok(raw_cmds[j], " \t\n");
        i = 0;
        while (token != NULL) {
            args[i] = strdup(token);
            token = strtok(NULL, " \t\n");
            i++;
        }
        args[i] = NULL;
        cmds[j] = args;
        if(args[0] == NULL){
            free_cmds(cmds, j+1);
            return -1;
        }
    }
    
    return count;
}