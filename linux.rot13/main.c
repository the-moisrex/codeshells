#include "tlpi_hdr.h"
#include "server.h"
#include "client.h"

#include <netdb.h>
#include <sys/socket.h>

enum app_type{
  app_server,
  app_client
};

int main(int argc, char** argv) {

  enum app_type type;
  if (argc <= 1) {
    type = app_server;
  } else if (argc >= 2 && strcmp(argv[1], "client") == 0) {
    type = app_client;
  } else {
    type = app_server;
  }


  if (type == app_server) { // server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2020);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (start_server((struct sockaddr*)&server, sizeof(server)) == -1) {
      errExit("Can't start the server.");
    }

  } else { // client

    struct hostent* host_info;
    struct sockaddr_in server;
    char* server_name;
    int port;

    if (argc == 4) {
      server_name = argv[2];
      port = strtol(argv[3], &argv[3] + strlen(argv[3]), 10);
    } else if (argc == 3) {
      server_name = argv[2];
      port = 2020;
    } else {
      server_name = "localhost";
      port = 2020;
    }

    host_info = gethostbyname(server_name);
    if (host_info == NULL) {
      perror("finding the host name specified");
      exit(1);
    }

    server.sin_family = AF_INET;
    memcpy((char*)&server.sin_addr, host_info->h_addr, host_info->h_length);
    server.sin_port = htons(port);

    start_client((struct sockaddr*)&server, sizeof server);
  }

  return 0;
}

