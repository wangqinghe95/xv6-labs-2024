#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find(char* dir, char* name)
{
    char buf[512];
    char* p;

    struct dirent de;
    struct stat st;

    int fd;
    if((fd = open(dir, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    if(st.type != T_DIR) {
        fprintf(2, "find: %s is not a directory\n", dir);
        close(fd);
        return;
    }

    strcpy(buf, dir);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if(de.inum == 0) continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0) {
            printf("ls: cannot stat %s\n", buf);
            continue;
        }

        if(st.type == T_DIR) {
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                continue;
            }

            find(buf, name);
        }

        if(strcmp(de.name, name) == 0) {
            printf("%s\n", buf);
        }
    }
    
}

int main(int argc, char* argv[])
{
    if(argc != 3) {
        printf("Usage: find <dir> <name>\n");
        exit(1);
    }

    char* dir = argv[1];
    char* name = argv[2];

    find(dir, name);

    exit(0);
}