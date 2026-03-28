#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

extern pid_t shell_pgid;

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
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        
        setpgid(0, 0);
        int pgid_coord = getpid();
        
        int prev_fd = -1;

        for(int i = 0; i < count; i++){
            int fd[2];
            if(i < count-1){
                if(pipe(fd) < 0){
                    perror("pipe"); 
                    exit(1); 
                }
            }

            pid_t child = fork();
            if(child < 0){ 
                perror("fork"); 
                exit(1); 
            }
            setpgid(child, pgid_coord);

            if(child == 0){
                setpgid(0, pgid_coord);
                if(prev_fd != -1){       
                    safe_dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }
                if(i < count-1){          
                    close(fd[0]);
                    safe_dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                }
                execute(cmds[i]);
                exit(0);
            }

            if(prev_fd != -1) close(prev_fd);
            if(i < count-1){
                close(fd[1]);
                prev_fd = fd[0]; 
            }
        }

        while(wait(NULL) > 0);  
        exit(0);
    }
    else{
        setpgid(pid, pid);
        
        if(!bg){
            tcsetpgrp(0, pid);
            sigset_t mask, old;
            sigemptyset(&mask);
            sigaddset(&mask, SIGCHLD);
            sigprocmask(SIG_BLOCK, &mask, &old);

            int status;
            waitpid(pid, &status, 0);
            if(WIFSIGNALED(status) && WTERMSIG(status) == SIGINT){
                write(STDOUT_FILENO, "\n", 1);
            }

            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            tcsetpgrp(0, shell_pgid);
        }
        
        free_cmds(cmds, count);
    }
    return 0;
}