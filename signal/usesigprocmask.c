#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sighandle(int s)
{
    printf("收到了SIGQUIT信号...\n");
}


int main(int argc, char* argv[])
{
    sigset_t newmask, oldmask;
    if (signal(SIGQUIT, sighandle) == SIG_ERR)
    {
        printf("无法收到SIGQUIT信号...、n");
        exit(1);
    }

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
    {
        perror("sigprocmask");
        exit(1);
    }

    printf("开始休息10s...\n");
    sleep(10);
    printf("已经休息10s...\n");

    if (sigismember(&newmask, SIGQUIT))
    {
        printf("SIGQUIT信号已经被屏蔽...\n");
    }
    else
    {
        printf("SIGQUIT信号没有被屏蔽...\n");
    }

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
    {
        perror("procmask");
        exit(1);
    }

    printf("sigprocmask()成功\n");

    if (sigismember(&oldmask, SIGQUIT))
    {
        printf("SIGQUIT信号被屏蔽了\n");
    }
    else
    {
        printf("SIGQUIT信号没有被屏蔽, 你可以发送SIGQUIT信号了, 接下来我要睡眠10s...\n");
        int mysl = sleep(10);
        if (mysl > 0)
        {
            printf("我还要睡眠%ds...\n", mysl);
        }
    }

    printf("再见！..\n");

    return 0;
}