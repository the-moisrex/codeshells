#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

void rot13(char *s) {
  char c;

  while (*s) {
    c = *s;
    if (c >= 'a' && c <= 'z') {
      c += 13;
      if (c > 'z')
        c -= 26;
    } else if (c >= 'A' && c <= 'Z') {
      c += 13;
      if (c > 'Z')
        c -= 26;
    }
    *s++ = c;
  }
}

int main() {
  int listenfd, connfd;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t len;
  char buf[MAXLINE];
  int n;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(listenfd, 10) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::cout << "Server listening on port " << PORT << std::endl;

  while (1) {
    len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
    if (connfd == -1) {
      perror("accept");
      continue;
    }

    std::cout << "Accepted new connection from " << inet_ntoa(cliaddr.sin_addr)
              << std::endl;

    int state = 1;
    if (setsockopt(connfd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state)) ==
        -1) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    while ((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
      rot13(buf);
      send(connfd, buf, n, 0);
    }

    state = 0;
    if (setsockopt(connfd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state)) ==
        -1) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    std::cout << "Closing connection from " << inet_ntoa(cliaddr.sin_addr)
              << std::endl;

    close(connfd);
  }

  return 0;
}
