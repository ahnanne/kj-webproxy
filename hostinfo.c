#include "csapp.h"

int main(int argc, char **argv) {
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    /* ./hostinfo twitter.com 를 입력할 경우 */
    printf("%d\n", argc); // 2
    printf("%s\n", argv[0]); // ./hostinfo
    printf("%s\n", argv[1]); // twitter.com

    /* addrinfo 레코드의 목록을 가져온다. */
    memset(&hints, 0, sizeof(struct addrinfo));
    // printf("%lu", sizeof(struct addrinfo)); // 48
    hints.ai_family = AF_INET; // IPv4로 제한
    hints.ai_socktype = SOCK_STREAM; // stream socket으로 제한 (connections only)

    // 인자로 넘기는 정보들을 소켓 주소 구조체(addrinfo)로 변환하기
    // 여기서 서비스 이름은 주소로 변환하지 않기로 한다. (NULL)
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp))) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    /* 리스트를 방문하면서 각각의 IP 주소를 표시한다. */
    flags = NI_NUMERICHOST; // 도메인 이름 대신 주소 문자열을 표시하기 위한 옵션 지정
    for (p = listp; p; p = p->ai_next) {
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf); // 호스트 이름 출력
    }

    /* 메모리 누수 방지 */
    Freeaddrinfo(listp);

    exit(0);
}
