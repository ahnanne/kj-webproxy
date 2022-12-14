#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const *user_agent_key = "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";

/**
 * 함수 선언
*/
void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_endserver(char *hostname, int port);

int main(int argc, char **argv)
{
  // proxy가 수신자의 입장에서 클라이언트 프로세스의 요청을 주시하기 위해 사용할 소켓
  int listenfd;
  // proxy가 수신자의 입장에서 클라이언트 프로세스와 통신하기 위해 사용할 소켓
  int connfd;

  socklen_t clientlen;
  char hostname[MAXLINE], port[MAXLINE];

  // 소켓 주소 범용 구조체인 sockaddr와 동일하게 취급되는 구조체
  struct sockaddr_storage clientaddr;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port> \n", argv[0]);

    // arguments가 불충분하므로 비정상 종료
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s:%s).\n", hostname, port);

    doit(connfd);
    Close(connfd);
  }

  return 0;
}

void doit(int connfd)
{
  // proxy가 송신자 입장에서 end server와 통신하기 위해 사용할 소켓
  // (end server - 클라이언트 프로세스가 연결을 원하는 서버 프로세스)
  int endserver_fd;
  int port;

  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char endserver_http_header[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE];

  // 클라이언트 쪽 rio
  rio_t rio;
  // end server 쪽 rio
  rio_t server_rio;

  // 클라이언트와..
  Rio_readinitb(&rio, connfd);
  rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);

  // 우선은 GET 요청만 처리하기로 함.
  if (strcasecmp(method, "GET")) {
    printf("Proxy does not implement the method.\n");
    return;
  }

  parse_uri(uri, hostname, path, &port);

  build_http_header(endserver_http_header, hostname, path, port, &rio);

  // end server에 연결하기 위한 소켓 생성
  endserver_fd = connect_endserver(hostname, port);

  if (endserver_fd == -1) {
    printf("connection failed\n");
    return;
  }

  // end server와..
  Rio_readinitb(&server_rio, endserver_fd);
  Rio_writen(endserver_fd, endserver_http_header, strlen(endserver_http_header));

  // end server로부터 응답을 받고, 이를 클라이언트로 전달
  size_t n;

  while (n = rio_readlineb(&server_rio, buf, MAXLINE)) {
    printf("proxy received %d bytes, then send\n", n);
    Rio_writen(connfd, buf, n);
  }

  Close(endserver_fd);
}

/**
 * end server에 전달할 HTTP 헤더를 생성하는 함수
*/
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio)
{
  char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];

  sprintf(request_hdr, "GET %s HTTP/1.0\r\n", path);

  // * 헤더 생성
  while (rio_readlineb(client_rio, buf, MAXLINE) > 0) {
    if (!strcmp(buf, endof_hdr)) {
      break;
    }

    // Host 헤더 생성
    if (!strncasecmp(buf, host_key, strlen(host_key))) {
      strcpy(host_hdr, buf);
      continue;
    }

    // Host 헤더 외의 헤더 생성
    if (
      !strncasecmp(buf, connection_key, strlen(connection_key))
      && !strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key))
      && !strncasecmp(buf, user_agent_key, strlen(user_agent_key))
    ) {
      strcat(other_hdr, buf);
    }
  }

  // req 헤더에 Host 헤더가 없다면, hostname을 값으로 하는 Host 헤더를 추가한다.
  if (!strlen(host_hdr)) {
    sprintf(host_hdr, "Host: %s\r\n", hostname);
  }

  // 위 반복문을 통해 생성한 개별 헤더들 및 기본 헤더들을 하나로 합치기
  sprintf(
    http_header, "%s%s%s%s%s%s%s",
    request_hdr, host_hdr,
    "Connection: close\r\n", "Proxy-Connection: close\r\n",
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n",
    other_hdr, endof_hdr
  );
}

/**
 * end server와 연결을 수립하는 함수
*/
inline int connect_endserver(char *hostname, int port)
{
  // atoi와 달리 itoa는 비표준이므로 다음과 같이 직접 변환해야 함.
  char *port_str[MAXLINE];
  sprintf(port_str, "%d", port);

  return Open_clientfd(hostname, port_str);
}

/**
 * URI를 파싱하는 함수
*/
void parse_uri(char *uri, char *hostname, char *path, int *port)
{
  // 포트번호가 누락되어 있을 경우에 대비하여 디폴트 포트번호 지정
  // ! 인바운드 규칙으로 허용해 둔 포트번호여야 함.
  // (클라이언트가 포트번호를 명시한 경우에는 아래 case 3에서 해당 포트번호로 덮어씌워짐.)
  *port = "57143";

  // strstr()으로 부분 문자열 찾아서 해당 문자열의 시작 주소를 가리키는 포인터 정의
  // (부분 문자열이 없다면 NULL을 반환)
  // ? https://man7.org/linux/man-pages/man3/strstr.3.html
  char *position1 = strstr(uri, "//"); // http://어쩌고 에서 "//" 요거
  position1 = position1 == NULL ? uri : position1 + 2; // IP 시작 위치

  char *position2 = strstr(position1, ":"); // ":포트번호" 시작 위치

  // printf("uri: %s\n", uri);
  // uri: http://13.124.89.247:57143/cgi-bin/adder?123&456

  if (position2 == NULL) {
    position2 = strstr(position1, "/");

    if (position2 == NULL) { /* case 1 */
      sscanf(position1, "%s", hostname);
    }
    else { /* case 2 */
      *position2 = '\0';
      sscanf(position1, "%s", hostname);

      *position2 = '/';
      sscanf(position2, "%s", path);
    }
  }
  /* case 3 */
  else { // 프로토콜과 포트번호가 모두 명시되어 있을 경우
    *position2 = '\0';
    sscanf(position1, "%s", hostname);
    sscanf(position2 + 1, "%d%s", port, path);
  }
}
