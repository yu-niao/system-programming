#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

const int CPS = 10;
const int BUFSIZE = CPS;
const int TOKENSIZE = 100;

static volatile int token = 0;

static void alrm_handler(int s)
{
    alarm(1);
    token++;
    if (token > TOKENSIZE)
        token = TOKENSIZE;
}

int main(int argc, char* argv[])
{
    int lfd = STDOUT_FILENO;
    int fd;
    int len, pos, ret;
    char buf[BUFSIZE];
    bzero(&buf, BUFSIZE);

    if (argc < 2)
    {
        printf("Usage: %s <filepath>\n", argv[0]);
        exit(1);
    }

    signal(SIGALRM, alrm_handler);
    alarm(1);

    do
    {
        fd = open(argv[1], O_RDONLY);
        if (fd < 0)
        {
            if (errno != EINTR)
            {
                perror("open");
                exit(1);
            }
        }
    }while(fd < 0);

   while(1)
   {
       while(!token)
           pause();

       token--;

       while ((len = read(fd, buf, BUFSIZE)) < 0)
       {
           if (errno == EINTR)
               continue;
           perror("read");
           break;
       }
       
       if (len == 0)
           break;

       pos = 0;
       while (len >0)
       {
           ret = write(lfd, buf + pos, len);
           if (ret < 0)
           {
               if (errno == EINTR)
                   continue;
               perror("write");
               exit(1);
           }
           pos += ret;
           len -= ret;
       }
   }

   close(fd);

    return 0;
}
