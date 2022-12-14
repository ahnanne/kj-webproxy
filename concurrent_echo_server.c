#include "csapp.h"

void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd;
    // 연결 소켓의 경우에는 피어 쓰레드로 전달해야 하므로, 연결 소켓을 가리키는 포인터를 선언
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);

        // 피어 쓰레드에 연결 소켓을 전달하기 위해,
        // 연결 소켓 생성 후 이를 가리키는 포인터를 쓰레드 생성 함수의 인자로 전달
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // pthread_create의 두 번째 인자로 NULL을 전달할 경우
        // 생성되는 쓰레드는 기본 특성을 가짐.
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);

    // 각 쓰레드를 분리하여, 각 쓰레드가 종료할 때 이들의 자원이 반환되도록 한다.
    // (쓰레드를 명시적으로 reap하지 않기 때문)
    Pthread_detach(pthread_self());
    Free(vargp); // 동적 할당했던 메모리 반환

    echo(connfd);
    Close(connfd);
}

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    //  두 번째 인자로 전달한 식별자를 버퍼와 연결

    // * robust I/O (Rio) package의 상태
    //   typedef struct {
    //     int rio_fd;                /* Descriptor for this internal buf */
    //     int rio_cnt;               /* Unread bytes in internal buf */
    //     char *rio_bufptr;          /* Next unread byte in internal buf */
    //     char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
    // } rio_t;

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { // 글자 수
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}
