/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1, n2;

  /**
   * 쿼리 스트링에서 2개의 인자 추출하기.
   * 
   * getenv 함수는 자신을 호출한 프로세스의 환경에서, 인자로 전달 받은 환경변수를 찾는다.
   * 만약 해당 환경변수가 존재한다면 그 환경변수의 값을 가리키는 포인터를 반환한다.
   * (이 시점에선 tiny.c의 parse_uri() 함수에서 환경변수 설정 작업을 거친 상태)
  */
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p = strchr(buf, '&');
    *p = '\0';

    strcpy(arg1, buf);
    strcpy(arg2, p + 1);

    /**
     * atoi - convert a string to an integer
    */
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /**
   * res body 생성하기
  */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "%s이것은 response body입니다..", content);
  sprintf(content, "%s입력하신 두 값을 더한 값은: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%s방문해주셔서 감사해요^^\r\n", content);

  /**
   * HTTP res 생성하기
  */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html; charset=utf-8\r\n\r\n");
  printf("%s", content); // 앞서 생성한 res body 출력

  /**
   * buffered stream을 flush한다.
   * 즉, 누적된 모든 문자를 파일로 전송한다.
  */
  fflush(stdout);

  exit(0);
}
/* $end adder */
