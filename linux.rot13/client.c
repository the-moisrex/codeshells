#include "./client.h"

#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int start_client(struct sockaddr* server, int server_size) {
  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if ( sock < 0 ) {
    perror("creating stream socket.");
    exit(1);
  }

  if (connect(sock, server, server_size) < 0) {
    perror("connecting to server");
    exit(2);
  }

  int count;
  char buf[1024];
  while ((count = read(0, buf, 1024)) > 0) {
    write(sock, buf, count);
    read(sock, buf, count);
    write(1, buf, count);
  }
}
