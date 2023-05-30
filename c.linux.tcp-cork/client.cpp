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

int main() {
  int sockfd;
  struct sockaddr_in servaddr;
  char sendline[MAXLINE], recvline[MAXLINE];
  int n;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  int state = 1;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  while (fgets(sendline, MAXLINE, stdin) != NULL) {
    send(sockfd, sendline, strlen(sendline), 0);
    if ((n = recv(sockfd, recvline, MAXLINE, 0)) == 0) {
      perror("server terminated prematurely");
      exit(EXIT_FAILURE);
    }
    recvline[n] = '\0';
    fputs(recvline, stdout);
  }

  state = 0;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  close(sockfd);

  return 0;
}
