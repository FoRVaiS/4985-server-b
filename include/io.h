#ifndef IO_H
#define IO_H

#include <unistd.h>

#define BUFFER_SIZE 4096

ssize_t read_fully(int fd, char *buf, size_t size, int *err);

ssize_t write_fully(int fd, void *buf, ssize_t size, int *err);

ssize_t copy(int from, int to, int *err);

#endif    // IO_H
