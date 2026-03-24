#ifndef UTILS_H
#define UTILS_H

void free_cmds(char ***cmds, int count);
void safe_dup2(int oldfd, int newfd);
int safe_open(const char *path, int flags, mode_t mode);

#endif