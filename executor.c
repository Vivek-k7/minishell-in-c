#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
    int file1 = -1;
    int file2 = -1;
    int file3 = -1;
    int i = 0;
    for(; args[i] != NULL; i++){
        if(strcmp(args[i], ">") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "syntax error: expected file after >\n");
                exit(1);
            }
            file1 = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        }
    }
    int j = 0;
    for(; args[j] != NULL; j++){
        if(strcmp(args[j], "<") == 0 && args[j+1] != NULL){
            file2 = open(args[j+1], O_RDONLY);
            break;
        }
    }
    int k = 0;
    for(; args[k] != NULL; k++){
        if(strcmp(args[k], ">>") == 0 && args[k+1] != NULL){
            file3 = open(args[k+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
            break;
        }
    }
    
    if(file1 != -1){
        args[i] = NULL;
        dup2(file1, 1);
        close(file1); 
    }

    if(file2 != -1){
        args[j] = NULL;
        dup2(file2, 0);
        close(file2); 
    }

    if(file3 != -1){
        args[k] = NULL;
        dup2(file3, 1);
        close(file3); 
    }
    execvp(args[0], args);
    perror("execvp");
    
}

void execute_pipeline(char ***cmds, int count, bool bg){

    pid_t  pid = fork();
        
    if(pid == 0){
        signal(SIGINT, SIG_DFL);
        rec(cmds, count-1);
    }
    else{
        if(!bg) waitpid(pid, NULL, 0);
        
        for(int j = 0; j < count; j++){
            free(cmds[j]);
        }
    }
}