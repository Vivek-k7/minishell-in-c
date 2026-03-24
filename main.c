#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "signals.h"
#include "parser.h"
#include "executor.h"

int main(){
    char cmd[256];
    char **cmds[64];

    setup_signals();
    
    while(1){
        errno = 0;
        printf("myshell> ");
        fflush(stdout);  
        if(!fgets(cmd, sizeof(cmd), stdin)){
            if(errno == EINTR){
                clearerr(stdin);
                continue;
            }
            else break;
        }

        char *p = cmd;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') continue;
        
        int count = parse_input(p, cmds);
        if(count == 0) continue;
        if(count <= 0){
            if(count == -1) fprintf(stderr, "syntax error\n");
            if(count == -2) fprintf(stderr, "system error\n");
            continue;
        }

        if(strcmp(cmds[0][0], "exit") == 0){
            for(int j = 0; j < count; j++){
                free(cmds[j]);
            }
            exit(0);
        }
        if(strcmp(cmds[0][0], "cd") == 0){
            if(cmds[0][1] != NULL){
                if(chdir(cmds[0][1]) != 0)
                    perror("cd");
            }
            for(int j = 0; j < count; j++){
                free(cmds[j]);
            }
            continue;
        }

        if(cmds[0][0] == NULL){
            for(int j = 0; j < count; j++) free(cmds[j]);
            continue;
        }

        bool bg = false;
        
        char **last = cmds[count-1];
        int k = 0;
        while(last[k] != NULL) k++;

        if(k > 0 && strcmp(last[k-1], "&") == 0){
            bg = true;
            last[k-1] = NULL;
        }

        if(execute_pipeline(cmds, count, bg) == -1) continue;

    }
}