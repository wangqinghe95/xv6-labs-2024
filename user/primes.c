#include "kernel/types.h"
#include "user.h"

void primes(int pipe_parent[2]) __attribute__((noreturn)); // __attribute__

void primes(int pipe_parent[2])
{
    int pipe_child[2];
    
    int read_prime = 0;

    close(pipe_parent[1]);      // close write end

    if(sizeof(int) == read(pipe_parent[0], &read_prime, sizeof(int))) {
        printf("prime %d\n", read_prime);
        if(pipe(pipe_child) < 0) {
            fprintf(2, "pipe failed!\n");
            exit(1);
        }

        if( 0 == fork()) {
            close(pipe_parent[0]);
            primes(pipe_child);
            exit(0);
        }
        else {
            close(pipe_child[0]);

            int n = 0;
            while (read(pipe_parent[0], &n, sizeof(int)) == sizeof(int))
            {
                if(n % read_prime != 0) {
                    write(pipe_child[1], &n, sizeof(int));
                }
            }
            close(pipe_child[1]);
        }
    }

    close(pipe_parent[0]);
    wait(0);
    exit(0);
}

int main()
{
    int pipe_parent[2];

    if(0 > pipe(pipe_parent)) {
        fprintf(2, "pipe error");
        exit(0);
    }

    if(0 == fork()) {
        // child process
        primes(pipe_parent);
    }
    else {
        close(pipe_parent[0]);  // close read pipe
        for(int i = 2; i < 280; ++i) {
            write(pipe_parent[1], &i, sizeof(int));
        }
        close(pipe_parent[1]);      // close write pipe after writing
        wait(0); 
    }
    exit(0);
}
