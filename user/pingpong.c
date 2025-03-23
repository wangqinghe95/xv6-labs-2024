#include "kernel/types.h"
#include "user/user.h"

int main()
{
    int pipe_fd_parent_to_child[2] = {0};
    int pipe_fd_child_to_parent[2] = {0};
    char buffer[1];
    if(-1 == pipe(pipe_fd_parent_to_child) || -1 == pipe(pipe_fd_child_to_parent)) {
        printf("pipe() error");
        exit(-1);
    }

    int res_pid = fork();
    if( 0 > res_pid) {
        printf("fork() error");
        exit(-1);
    }
    else if(0 == res_pid) {
        close(pipe_fd_child_to_parent[0]);
        close(pipe_fd_parent_to_child[1]);

        read(pipe_fd_parent_to_child[0], buffer, 1);
        if(buffer[0] == 'i') {
            printf("%d: received ping\n", getpid());
        }

        buffer[0] = 'o';
        write(pipe_fd_child_to_parent[1], buffer, 1);
        close(pipe_fd_child_to_parent[1]);
        close(pipe_fd_parent_to_child[0]);

        exit(0);
    }
    else{
        close(pipe_fd_child_to_parent[1]);
        close(pipe_fd_parent_to_child[0]);

        buffer[0] = 'i';

        write(pipe_fd_parent_to_child[1], buffer, 1);

        read(pipe_fd_child_to_parent[0], buffer, 1);
        if(buffer[0] == 'o') {
            printf("%d: received pong\n", getpid());
        }

        close(pipe_fd_child_to_parent[0]);
        close(pipe_fd_parent_to_child[1]);

        wait(0);
        exit(0);
    }

    exit(0);
}