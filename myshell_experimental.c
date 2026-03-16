#include<stdio.h>

int main(){
    char cmd[256];
    while(1){
        printf("myshell> ");
        fgets(cmd, sizeof(cmd), stdin);
    }
}