#include "kernel/types.h"
#include "user.h"

int main()
{
    fprintf(2, "uptime: %d\n", uptime());
    return 0;
}