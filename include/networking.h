#ifndef NETWORKING_H
#define NETWORKING_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

ssize_t convert_port(const char *str, in_port_t *port);
int     tcp_server(const char *address, in_port_t port, int backlog, int *err);
int     tcp_client(const char *address, in_port_t port, int *err);
int     setSocketNonBlocking(int socket, int *err);

#endif    // NETWORKING_H
