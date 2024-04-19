#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define LOGIN "login:ser_to_p1"
#define QUIT "quit:ser_to_p1"
#define USERID 1

typedef struct Message
{
    int usrId;
    char buf[100];
} Msg;

int main()
{
    int fd_w = open("major", O_WRONLY); // 向公共管道中写数据的fd_w
    Msg msg_login;
    msg_login.usrId = USERID;
    strcpy(msg_login.buf, LOGIN);
    write(fd_w, (char *)&msg_login, sizeof(msg_login));
    printf("login success\n");

    pid_t pid;
    pid = fork();
    if (pid > 0)
    {
        char buf[1024];
        bzero(buf, sizeof(buf));
        int fd_r = open("ser_to_p1", O_RDONLY); // 读取数据的fd_r
        printf("open ser_to_p1 success\n");
        //  读取管道中是否有信息
        int len;
        while (1)
        {
            len = read(fd_r, buf, sizeof(buf));
            if (len > 0) // 若有数据
            {
                printf("recever data: %s\n", buf);
            }
        }
    }
    else if (pid == 0)
    {
        usleep(100); // 防止  请输入指令： 与 recever 重合
        char *op = NULL;
        char buf[1024];

        while (1)
        {
            usleep(100); // 防止  请输入指令： 与 recever 重合
            // 输入你想要的指令
            printf("请输入指令：");
            bzero(buf, sizeof(buf));
            scanf("%s", buf);
            if (strcmp(buf, "quit") == 0)
            {
                write(fd_w, QUIT, strlen(QUIT));
                printf("send quit success\n");
                unlink("ser_to_p1");
                close(fd_w);
                exit(0);
            }
            else if (strcmp(buf, "all") == 0)
            {
                write(fd_w, "all", sizeof("all"));
                printf("wait all success\n");
            }
            else
            {
                if (buf[0] == 'p')
                {
                    Msg msg;
                    msg.usrId = USERID;
                    strcpy(msg.buf, buf);
                    write(fd_w, (char *)&msg, sizeof(msg));
                    printf("write success \n");
                }
                else
                {
                    printf("data error \n");
                    continue;
                }
            }
        }
    }
    return 0;
}
