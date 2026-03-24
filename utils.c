#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void free_cmds(char ***cmds, int count){
    for(int i = 0; i < count; i++){
        if(cmds[i] == NULL) continue;

        for(int j = 0; cmds[i][j] != NULL; j++){
            free(cmds[i][j]); 
        }
        free(cmds[i]);
    }
}

void safe_dup2(int oldfd, int newfd){
    if(dup2(oldfd, newfd) < 0){
        perror("dup2");
        exit(1);
    }
}

int safe_open(const char *path, int flags, mode_t mode){
    int fd = open(path, flags, mode);
    if(fd < 0){
        perror("open");
        exit(1);
    }
    return fd;
}