#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main()
{
    pid_t pid;
    puts("begin:");

    fflush(NULL); //进行fork之前刷新一下缓冲区，防止数据遗漏

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        //用vim打开家目录下prosh.sh文件
        execl("/bin/vim", "vim", "/home/yu-niao/proxy.sh", NULL);
        perror("execl");
        exit(1);
    }
    
    wait(NULL);

    puts("end:");

    return 0;
}
