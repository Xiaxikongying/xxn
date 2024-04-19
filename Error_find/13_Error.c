#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

int status; // 0表示当前需要读取    1表示当前是判断   2表示当前是写入到result
char bufferA[1024];
char bufferB[1024];
pthread_cond_t cd0;
pthread_cond_t cd1;
pthread_cond_t cd2;

pthread_mutex_t lock;
pthread_mutex_t lock2;
FILE *fd_r;
FILE *fd_w;

void *pthread_read(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (status != 0)
            pthread_cond_wait(&cd0, &lock);

        // 读取一行数据  写入buffer A  唤醒2
        bzero(bufferA, sizeof(bufferA));
        if (fgets(bufferA, sizeof(bufferA), fd_r) == NULL) // 如果读取完成就exit
            exit(0);
        printf("%s\n", bufferA);
        status = 1;
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&cd1); // 唤醒1
    }
}
void *pthread_is(void *arg)
{
    // 判断bufferA中是不是CHIUSECASE错误
    // 是就写入bufferB唤醒3   不是先情况buffA 就唤醒1继续read
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (status != 1)
            pthread_cond_wait(&cd1, &lock);

        printf("is\n");
        if (strstr(bufferA, "CHIUSECASE") == NULL)
        {
            printf("no\n");
            // 不是CHIUSECASE继续执行0
            status = 0;
            pthread_mutex_unlock(&lock);
            pthread_cond_signal(&cd0); // 唤醒0
        }
        else // 查找成功  写入buffB
        {
            printf("yes\n");
            strcpy(bufferB, bufferA);
            status = 2;
            pthread_mutex_unlock(&lock);
            pthread_cond_signal(&cd2); // 唤醒2
        }
    }
}
void *pthread_write(void *arg)
{
    // 从bufferB中读取数据，写入Result.log
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (status != 2)
            pthread_cond_wait(&cd2, &lock);

        printf("write: %s\n", bufferB);
        fputs(bufferB, fd_w);
        bzero(bufferB, sizeof(bufferB));
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&cd0); // 唤醒0
    }
}

int main()
{
    fd_r = fopen("error", "rt");
    fd_w = fopen("result", "at");
    if (fd_r == NULL || fd_w == NULL)
        printf("无法打开\n");

    // 创建线程
    pthread_t tid[3];
    pthread_create(&tid[0], NULL, pthread_read, NULL);
    pthread_create(&tid[1], NULL, pthread_is, NULL);
    pthread_create(&tid[2], NULL, pthread_write, NULL);
    while (1)
        sleep(1);
    return 0;
}