#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

void execute(char **args);
void rec(char ***cmds, int i);

void rec(char*** cmds, int i){
    if(i == 0){
        execute(cmds[i]);
        return;
    }
    int fd[2];    
    if(pipe(fd) < 0){
        perror("pipe");
        exit(1);
    }

    pid_t pid1 = fork();

    if(pid1 < 0){
        perror("fork");
        exit(1);
    }

    if(pid1 == 0){
        close(fd[0]);
        safe_dup2(fd[1], 1);
        close(fd[1]);
        rec(cmds, i-1);
        exit(0);
    }
    else{   
        pid_t pid2 = fork();
        if(pid2 == 0){
            close(fd[1]);
            safe_dup2(fd[0], 0);
            close(fd[0]);
            execute(cmds[i]);
        }
        else{
            close(fd[0]);
            close(fd[1]);
            while(wait(NULL) > 0);
        }   
    }
}

void execute(char **args){
    int infile = -1, outfile = -1;

    char *new_args[64];
    int ni = 0;

    for(int i = 0; args[i] != NULL; i++){
        if(strcmp(args[i], "<") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "syntax error: expected file after <\n");
                exit(1);
            }
            infile = safe_open(args[++i], O_RDONLY, 0);
        }
        else if(strcmp(args[i], ">") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "syntax error: expected file after >\n");
                exit(1);
            }
            outfile = safe_open(args[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else if(strcmp(args[i], ">>") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "syntax error: expected file after >>\n");
                exit(1);
            }
            outfile = safe_open(args[++i], O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else{
            new_args[ni++] = args[i];
        }
    }

    new_args[ni] = NULL;

    if(infile != -1){
        safe_dup2(infile, STDIN_FILENO);
        close(infile);
    }

    if(outfile != -1){
        safe_dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }

    execvp(new_args[0], new_args);
    perror("execvp");
    exit(1);
}

int execute_pipeline(char ***cmds, int count, bool bg){

    pid_t  pid = fork();
    
    if (pid < 0) {
        perror("fork error"); 
        free_cmds(cmds, count);
        return -1;
    }

    if(pid == 0){
        signal(SIGINT, SIG_DFL);
        rec(cmds, count-1);
        exit(0);
    }
    else{
        if(!bg) while (wait(NULL) > 0);
        free_cmds(cmds, count);
    }
    return 0;
}