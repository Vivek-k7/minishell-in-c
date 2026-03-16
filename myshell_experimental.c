#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

void execute(char **args);
void rec(char ***cmds, int i);

void rec(char*** cmds, int i){
    if(i == 0){
        execute(cmds[i]);
        return;
    }
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if(pid == 0){
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        rec(cmds, i-1);
    }
    else{   
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        execute(cmds[i]);
    }
}

void execute(char** args){
    int file = -1;
    int i = 0;
    for(; args[i] != NULL; i++){
        if(strcmp(args[i], ">") == 0){
            file = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        }
    }
    
    if(file != -1){
        args[i] = NULL;
        dup2(file, 1);
        close(file); 
    }
    execvp(args[0], args);
    perror("execvp");
    
}

int main(){
    char cmd[256];
    char **cmds[64];
    
    while(1){
        printf("myshell> ");
        fgets(cmd, sizeof(cmd), stdin);
        int i = 0;
        char *raw_cmds[64];

        char* token = strtok(cmd, "|");
        while (token != NULL) {
            raw_cmds[i] = token;
            token = strtok(NULL, "|");
            i++;
        }
        raw_cmds[i] = NULL;
        bool bg = false;
        int count = i;
        if(raw_cmds[0] == NULL) continue;
        for(int j = 0; raw_cmds[j]!=NULL; j++){
            char *args = malloc(sizeof(char) * 64);
            token = strtok(raw_cmds[j], " \t\n");
            i = 0;
            while (token != NULL) {
                args[i] = token;
                token = strtok(NULL, " \t\n");
                i++;
            }
            args[i] = NULL;
            cmds[j] = args;
        }
        char **last = cmds[count-1];
        int k = 0;
        while(last[k] != NULL) k++;

        if(k > 0 && strcmp(last[k-1], "&") == 0){
            bg = true;
            last[k-1] = NULL;
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
        pid_t  pid = fork();
        
        if(pid == 0){
            rec(cmds, count-1);
        }
        else{
            if(!bg) wait(NULL);
            
            for(int j = 0; j < count; j++){
                free(cmds[j]);
            }
        }
    }
}