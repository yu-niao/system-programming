#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENT_NUM 1024

static int pipefd[2];

int setnonblock(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

void addfd(int epollfd, int fd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;

    int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    assert(ret != -1);

    setnonblock(fd);
}

void handler(int s)
{
    int save_errno = errno;
    send(pipefd[1], (char*)&s, 1, 0);
    errno = save_errno;
}

void addsig(int s)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigfillset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags |= SA_RESTART;
    assert(sigaction(s, &sa, NULL) != -1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s, ip, port.\n", argv[0]);
        exit(1);
    }

    addsig(SIGINT);
    addsig(SIGTERM);
    addsig(SIGQUIT);
    

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(serv_sock != -1);

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr.s_addr);
    int ret = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(ret != -1);

    ret = listen(serv_sock, 5);
    assert(ret != -1);

    int epollfd = epoll_create(5);

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd) != -1);
    setnonblock(pipefd[1]);
    addfd(epollfd, pipefd[0]);
    int serv_stop = 0;

    struct epoll_event events[MAX_EVENT_NUM];
    addfd(epollfd, serv_sock);

    while(!serv_stop)
    {
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
        if ((num == -1) && (errno != EINTR))
        {
            printf("epoll_wait()\n");
            break;
        }

        for (int i = 0; i < num; i++)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == serv_sock)
            {
                struct sockaddr_in clit_addr;
                socklen_t clit_len = sizeof(clit_addr);
                int confd = accept(sockfd, (struct sockaddr*)&clit_addr, &clit_len);
                assert(confd != -1);
                addfd(epollfd, confd);
            }
            else if ((sockfd == pipefd[0]) && (events[i].events && EPOLLIN))
            {
                char sigs[1024];
                ret = recv(pipefd[0], sigs, sizeof(sigs), 0);
                if (ret < 0)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0; i < ret; i++)
                    {
                        switch(sigs[i])
                        {
                            case SIGINT:
                            {
                                serv_stop = 1;
                                printf("\nbegin disconnecting...\n");
                            }
                            case SIGTERM:
                            {}
                            case SIGQUIT:
                            {}
                            default:
                            continue;
                        }
                    }
                }
            }
            else
            {

            }
        }
    }
    printf("close fds...\n");
    close(serv_sock);
    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}