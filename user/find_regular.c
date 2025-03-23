#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char g_buf[1024];
int match(char*, char*);
void grep(char *pattern, int fd);

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

        if(match(de.name, name)) {
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

int matchhere(char*, char*);
int matchstar(int, char*, char*);

void grep(char *pattern, int fd)
{
  int n, m;
  char *p, *q;

  m = 0;
  while((n = read(fd, g_buf+m, sizeof(g_buf)-m-1)) > 0){
    m += n;
    g_buf[m] = '\0';
    p = g_buf;
    while((q = strchr(p, '\n')) != 0){
      *q = 0;
      if(match(pattern, p)){
        *q = '\n';
        write(1, p, q+1 - p);
      }
      p = q+1;
    }
    if(m > 0){
      m -= p - g_buf;
      memmove(g_buf, p, m);
    }
  }
}


int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}