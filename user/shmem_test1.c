#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int pipe_fd[2], pid;
    #define MSG_LEN 20
    
    pipe(pipe_fd);

    if((pid = fork()) == 0) {
        close(pipe_fd[1]);
        char *dst_va = 0;
        read(pipe_fd[0], &dst_va, MSG_LEN);
        printf("Child: %s\n", dst_va);
        close(pipe_fd[0]);
    } else {
        close(pipe_fd[0]);
        char *shmem = (char*) malloc(MSG_LEN*sizeof(char));
        char *dst_va = map_shared_pages(pid, shmem, MSG_LEN);
        strcpy(shmem, "Hello child");
        write(pipe_fd[1], &dst_va, MSG_LEN);
        close(pipe_fd[1]);
        wait(0);
    }
    exit(0);
}
