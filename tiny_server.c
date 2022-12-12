/**
 * tiny server - 단순한 iterative HTTP/1.0 웹 서버
 * GET 메서드를 통해 정적 및 동적 컨텐츠를 제공합니다.
*/
#include "csapp.h"

int main(int argc, char **argv)
{
    // TODO:
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
