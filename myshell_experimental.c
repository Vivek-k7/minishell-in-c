#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(){
    char cmd[256];
    char *args[64];
    while(1){
        printf("myshell> ");
        fgets(cmd, sizeof(cmd), stdin);
        int i = 0;
        char* token = strtok(cmd, " \t\n");
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " \t\n");
            i++;
        }

        args[i] = NULL;
        if(args[0] == NULL) continue;
        
        pid_t  pid = fork();
        
        if(pid == 0){
            execvp(args[0], args);
            perror("execvp");
        }
        else{
            wait(NULL);
        }
    }
}