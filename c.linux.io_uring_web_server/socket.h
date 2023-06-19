//
// Created by moisrex on 6/16/23.
//

#ifndef C_LINUX_IO_URING_WEB_SERVER_SOCKET_H
#define C_LINUX_IO_URING_WEB_SERVER_SOCKET_H


#include <netinet/in.h>
#include <string.h>

/**
 * This function is responsible for setting up the main listening socket used by
 * the web server.
 */
int setup_listening_socket(int port) {
    int sock;
    struct sockaddr_in srv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        fatal_error("socket()");

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        fatal_error("setsockopt(SO_REUSEADDR)");

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* We bind to a port and turn this socket into a listening
   * socket.
    */    if (bind(sock, (const struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
        fatal_error("bind()");

    if (listen(sock, 10) < 0)
        fatal_error("listen()");

    return (sock);
}


#endif // C_LINUX_IO_URING_WEB_SERVER_SOCKET_H
