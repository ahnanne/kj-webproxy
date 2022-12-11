#include "csapp.h"

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

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        prinf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}
