#include "./server.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>



void rot13(unsigned char* buf, int count) {
  unsigned char* buf_end = buf + count;
  for (; buf != buf_end; ++buf) {
    if (islower(*buf)) {
      *buf += 13;
      if (*buf > 'z') {
        *buf -= 26;
      }
    }
  }
}

void rot13_service(int in, int out) {
  unsigned char buf[1024];
  int count;
  while ((count = read(in, buf, 1024)) > 0) {
    rot13(buf, count);
    write(out, buf, count);
    write(1, buf, count);
  }
}

int start_server(struct sockaddr* server, int server_size) {
  if (server == NULL)
    return -1;

  int sock, fd, client_length;
  struct sockaddr_in client;
  struct servent * service;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return -1;
  }
  printf("Got a new socket.\n");

  if (bind(sock, server, server_size) == -1) {
    return -1;
  }
  printf("Bind works as well.\n");

  if (listen(sock, 5) == -1) {
    return -1;
  }
  printf("Listening...\n");

  while(1) {
    client_length = sizeof(client);
    fd = accept(sock, (struct sockaddr*)&client, &client_length);
    if (fd == -1) {
      printf("Couldn't accept the connection.");
      continue;
    }
    printf("Got connected");
    rot13_service(fd, fd);
    if (close(fd) == -1) {
      printf("Connection didn't close properly.");
    }
  }

  close(sock);

}
