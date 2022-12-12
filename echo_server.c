#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    /**
     * protocol-independent
     * 즉, 어떤 형태의 소켓 주소든 저장할 수 있을 정도로 크기가 충분히 크다.
    */
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /**
     * 내부적으로 getaddrinfo, socket, bind, listen을 실행하고
     * listening socket을 반환한다.
    */
    listenfd = Open_listenfd(argv[1]);

    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("Connected to (%s:%s)\n", client_hostname, client_port);

    echo(connfd);
    Close(connfd);
    printf("클라이언트 프로세스(%s:%s)에서 연결을 종료했습니다.\n", client_hostname, client_port);

    exit(0);
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
