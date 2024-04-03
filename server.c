#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 10
#define CALL_ALL "everyone hello!!"

typedef struct Message
{
    int usrId;
    char buf[100];
} Msg;

int fd[MAX];
int main()
{
    fd[0] = open("major", O_RDONLY);
    int i = 1;
    char buf[1024]; // 接受各种信息
    char *op = NULL;
    while (1)
    {
        bzero(buf, sizeof(buf));
        read(fd[0], buf, sizeof(buf));
        op = strtok(buf, ":");
        if (strcmp(op, "quit") == 0)
        {
            // 删除管道  quit:pipename
            op = strtok(NULL, ":");
            printf("pipe [%s] will be closed\n", op);
        }
        else if (strcmp(buf, "all") == 0)
        {
            // 反馈所有用户
            for (int i = 1; i <= MAX; i++)
            {
                if (fd[i] == 0)
                    continue;
                write(fd[i], CALL_ALL, strlen(CALL_ALL));
            }
        }
        else // 接受用户发送信息(报告login 与  聊天信息)
        {
            Msg *msg = (Msg *)buf;
            char tbuf[1024];
            strcpy(tbuf, msg->buf); // 防止msg->buf被破坏
            printf("write data: %s\n", msg->buf);
            op = strtok(tbuf, ":");

            if (strcmp(op, "login") == 0)
            {
                op = strtok(NULL, ":");
                // 创建新管道
                mkfifo(op, 0777);
                fd[msg->usrId] = open(op, O_WRONLY);
                printf("make pipe %d success\n", msg->usrId);
            }
            else if (msg->buf[0] == 'p')
            {
                printf("---------\n");
                int dest = msg->buf[1] - '0'; // 获取目标
                msg->buf[1] = (char)msg->usrId + '0';
                if (fd[dest] == 0)
                {
                    printf("用户 [%d] 不存在 \n", dest);
                    continue;
                }
                write(fd[dest], msg->buf, strlen(msg->buf));
            }
        }
    }
    return 0;
}
/*
 if (strcmp(op, "login") == 0)
        {
            op = strtok(NULL, ":");
            // 创建新管道
            mkfifo(op, 0777);
            fd[i++] = open(op, O_WRONLY);
            printf("make pipe %d success\n", i - 1);
        }
        else
*/