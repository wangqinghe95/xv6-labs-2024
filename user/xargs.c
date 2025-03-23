#include "kernel/types.h"
#include "kernel/param.h"
#include "user.h"

int main(int argc, char* argv[])
{
    char buf[512];

    read(0, buf, sizeof(buf));

    char* args_new[MAXARG];
    for(int i = 1; i < argc; ++i) {
        args_new[i-1] = argv[i];
    }

    int arg_index = argc - 1;
    char* p = buf;
    while (arg_index < MAXARG && *p != '\0')
    {
       args_new[arg_index] = p;
       arg_index++;

       while (*p != '\n')
       {
            p++;
       }

       *p = 0;

       if(0 == fork()) {
            exec(args_new[0], args_new);
       }

       wait(0);

       arg_index = argc - 1;
       p++;
    }

    exit(0);
}