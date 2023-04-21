#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>

#define FNAME "/tmp/out"

int daemonize()
{
    switch(fork())
    {
    case -1:
        return -1;
    case 0:
        break;
    default:
        exit(1);
    }

    if (setsid() == -1)
        return -1;

    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
        return -1;

    if (dup2(fd, STDIN_FILENO) == -1)
        return -1;

    if (dup2(fd, STDOUT_FILENO) == -1)
        return -1;

    if (fd > 2)
        close(fd);

    chdir("/");

    return 1;
}

int main()
{
    FILE* fp;
    int i;

    openlog("mydaemon", LOG_PID, LOG_DAEMON);

    if (daemonize() != 1)
    {
        syslog(LOG_ERR, "守护进程创建失败...");
        return 1;
    }

    syslog(LOG_INFO, "守护进程创建成功,进程ID为: %d", getpid());

    fp = fopen(FNAME, "w+");
    if (fp == NULL)
    {
        syslog(LOG_ERR, "打开文件失败");
        return 1;
    }

    for (i = 1; ; i++)
    {
        fprintf(fp, "this is %d...\n", i);
        fflush(fp); //非常重要
        sleep(1);
    }

    fclose(fp);
    closelog();

    return 0;
}