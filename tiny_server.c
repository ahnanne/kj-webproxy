/**
 * tiny server - 단순한 iterative HTTP/1.0 웹 서버
 * GET 메서드를 통해 정적 및 동적 컨텐츠를 제공합니다.
*/
#include "csapp.h"

/**
 * 함수 선언
*/
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void get_filetype(char *filename, char *filetype);
void serve_static(int fd, char *filename, int filesize);
void serve_dynamic(int fd, char *filename, char *cgiargs);

int main(int argc, char **argv)
{
    // TODO:
}

/**
 * 클라이언트에게 에러 메시지를 보내는 함수
*/
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /**
     * HTTP res body 생성
    */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""fafafa"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /**
    * HTTP res 출력
   */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/**
 * request header를 읽는 함수
*/
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);

    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
}

/**
 * 정적 컨텐츠와 동적 컨텐츠 중 어느 것을 제공해야 하는지 판별하기 위해
 * HTTP URI를 분석하는 함수
*/
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    /* 정적 컨텐츠에 대한 요청일 경우 */
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");

        /**
         * 상대경로로 변환하는 작업
        */
        strcpy(filename, ".");
        strcat(filename, uri);

        /**
         * uri가 '/'로 끝난다면?
         * (이때 이 '/'는 클라이언트 단에서 붙여서 보내준 것)
        */
        if (uri[strlen(uri) - 1] == '/') {
            // default filename은 ./home.html이라고 가정한 상태
            strcat(filename, "home.html");
        }

        return 1;
    }

    /* 동적 컨텐츠에 대한 요청일 경우 */
    else {
        ptr = index(uri, '?');

        // 쿼리 스트링이 존재한다면
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else {
            strcpy(cgiargs, "");
        }

        strcpy(filename, ".");
        strcat(filename, uri);

        return 0;
    }
}

/**
 * 밑에서 등장할 serve_static에 대한 헬퍼 함수:
 * 파일 이름으로부터 파일 유형을 알아낸다.
*/
void get_filetype(char *filename, char *filetype)
{
    char *p;
    char extension[10];

    p = strchr(filename, '.');
    strcpy(extension, p);

    if (extension == ".html") {
        strcpy(filetype, "text/html");
    }
    else if (extension == ".gif") {
        strcpy(filetype, "image/gif");
    }
    else if (extension == ".png") {
        strcpy(filetype, "image/png");
    }
    else if (extension == ".jpg") {
        strcpy(filetype, "image/jpeg");
    }
    else {
        strcpy(filetype, "text/plain");
    }
}

/**
 * 정적 컨텐츠를 클라이언트에게 제공한다.
 * 서버의 디스크에서 파일을 가져요고
 * 이것을 클라이언트에게 돌려주는 방식으로 처리한다.
*/
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    // 요청 파일 확장자 알아내기
    get_filetype(filename, filetype);

    /**
     * 우선은 클라이언트 쪽에 성공을 알리는 응답 라인을, 서버 헤더와 함께 보낸다.
    */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);

    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    /**
     * 클라이언트에게 res body 전송
    */

    // opens filename and gets its descriptor
    srcfd = Open(filename, O_RDONLY, 0);
    // 요청 파일을 가상 메모리 영역에 매핑
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);

    // 매핑 완료해서 더 이상 필요 없으니 메모리 누수 방지를 위해 파일 닫기
    Close(srcfd);

    // 실질적으로 클라이언트에게 파일을 전송하는 부분
    Rio_writen(fd, srcp, filesize);

    // free 동적 할당 영역
    Munmap(srcp, filesize);
}

/**
 * 동적 컨텐츠를 클라이언트에게 제공한다.
 * 자식 프로세스의 컨텍스트에서 CGI 프로그램을 로드하고 실행하며
 * 그 출력을 클라이언트로 리턴하여 처리한다.
*/
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /**
     * 우선은 클라이언트 쪽에 성공을 알리는 응답 라인을, 서버 헤더와 함께 보낸다.
    */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /**
     * 호출 프로세스를 복제한 자식 프로세스를 생성한다.
    */
    if (Fork() == 0) {
        // setenv — 환경변수를 추가하거나 수정한다.
        setenv("QUERY_STRING", cgiargs, 1);

        // dup2 - duplicate an open file descriptor
        // redirect stdout to client
        Dup2(fd, STDOUT_FILENO); /* STDOUT_FILENO - standard output file descriptor */

        // execve - 첫 번째 인자로 전달된 파일 경로에 해당하는 파일을 실행 시킨다.
        Execve(filename, emptylist, environ); /* environ - Defined by libc */
        // C에서는 이와 같이 environ이라는 전역 변수가 미리 만들어져 있으며 이를 통해 환경 변수 목록을 확인할 수 있다.
        // 참고: https://ehpub.co.kr/tag/environ-%EB%B3%80%EC%88%98/
    }

    /**
     * wait - 자식 프로세스가 멈추거나 종료하는 것을 기다린다.
     * 즉, 호출 프로세스가 자신의 자식 프로세스의 종료 전까지 실행을 중단한다. (blocked)
    */
    Wait(NULL);
}
