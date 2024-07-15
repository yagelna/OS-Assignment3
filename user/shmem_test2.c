#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pipe_fd[2], pid;
    #define MSG_LEN 20

    if(pipe(pipe_fd) != 0){
        printf("pipe() failed\n");
        exit(1);
    }

    if((pid = fork()) == 0) { // Child
        printf("Child size before mapping %d\n", sbrk(0));
        char *dst_va = 0;
        close(pipe_fd[1]);
        read(pipe_fd[0], &dst_va, MSG_LEN*sizeof(char *));
        printf("Child size after mapping %d\n", sbrk(0));
        strcpy(dst_va, "Hello daddy");
        sleep(20);
        unmap_shared_pages(dst_va, MSG_LEN*sizeof(char));
        printf("Child size after unmapping %d\n", sbrk(0));
        close(pipe_fd[0]);
    } else {
        sleep(5);
        close(pipe_fd[0]);
        char *msg = (char*) malloc(MSG_LEN*sizeof(char));
        char *addr = map_shared_pages(pid, msg, MSG_LEN*sizeof(char));
        write(pipe_fd[1], &addr, MSG_LEN* sizeof(char *));
        sleep(10);
        printf("Parent got message from child: %s\n", msg);

        close(pipe_fd[1]);
        wait(0);
    }
    exit(0);
}
