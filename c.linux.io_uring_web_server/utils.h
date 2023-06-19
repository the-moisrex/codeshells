//
// Created by moisrex on 6/16/23.
//

#ifndef C_LINUX_IO_URING_WEB_SERVER_UTILS_H
#define C_LINUX_IO_URING_WEB_SERVER_UTILS_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_STRING "Server: zerohttpd/0.1\r\n"
#define DEFAULT_SERVER_PORT 8000
#define QUEUE_DEPTH 256
#define READ_SZ 8192

#define EVENT_TYPE_ACCEPT 0
#define EVENT_TYPE_READ 1
#define EVENT_TYPE_WRITE 2

/**
 * Utility function to convert a string to lower case.
 */
void strtolower(char* str) {
    for (; *str; ++str)
        *str = (char)tolower(*str);
}
/*
 One function that prints the system call and the error details
 and then exits with error code 1. Non-zero meaning things didn't go well.
 */
void fatal_error(const char* syscall) {
    perror(syscall);
    exit(1);
}

/*
 * Helper function for cleaner looking code.
  */
void* zh_malloc(size_t size) {
    void* buf = malloc(size);
    if (!buf) {
        fprintf(stderr, "Fatal error: unable to allocate memory.\n");
        exit(1);
    }
    return buf;
}


int get_line(const char* src, char* dest, int dest_sz) {
    for (int i = 0; i < dest_sz; i++) {
        dest[i] = src[i];
        if (src[i] == '\r' && src[i + 1] == '\n') {
            dest[i] = '\0';
            return 0;
        }
    }
    return 1;
}


#endif // C_LINUX_IO_URING_WEB_SERVER_UTILS_H
