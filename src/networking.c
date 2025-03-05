#include "networking.h"
#include <errno.h>
#include <fcntl.h>
#include <p101_c/p101_stdio.h>
#include <p101_c/p101_stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define ERR_NONE 0
#define ERR_NO_DIGITS 1
#define ERR_OUT_OF_RANGE 2
#define ERR_INVALID_CHARS 3

static void setup_network_address(struct sockaddr_storage *addr, socklen_t *addr_len, const char *address, in_port_t port, int *err);
static int  setup_tcp_server(const struct sockaddr_storage *addr, socklen_t addr_len, int backlog, int *err);
static int  connect_to_server(struct sockaddr_storage *addr, socklen_t addr_len, int *err);

int tcp_server(const char *address, in_port_t port, int backlog, int *err)
{
    struct sockaddr_storage addr;
    socklen_t               addr_len;
    int                     fd;

    setup_network_address(&addr, &addr_len, address, port, err);

    if(*err != 0)
    {
        fd = -1;
        goto done;
    }

    fd = setup_tcp_server(&addr, addr_len, backlog, err);

done:
    return fd;
}

int tcp_client(const char *address, in_port_t port, int *err)
{
    struct sockaddr_storage addr;
    socklen_t               addr_len;
    int                     fd;

    setup_network_address(&addr, &addr_len, address, port, err);

    if(*err != 0)
    {
        fd = -1;
        goto done;
    }

    fd = connect_to_server(&addr, addr_len, err);

done:
    return fd;
}

static void setup_network_address(struct sockaddr_storage *addr, socklen_t *addr_len, const char *address, in_port_t port, int *err)
{
    in_port_t net_port;

    *addr_len = 0;
    net_port  = htons(port);
    memset(addr, 0, sizeof(*addr));

    if(inet_pton(AF_INET, address, &(((struct sockaddr_in *)addr)->sin_addr)) == 1)
    {
        struct sockaddr_in *ipv4_addr;

        ipv4_addr           = (struct sockaddr_in *)addr;
        addr->ss_family     = AF_INET;
        ipv4_addr->sin_port = net_port;
        *addr_len           = sizeof(struct sockaddr_in);
    }
    else if((inet_pton(AF_INET6, address, &(((struct sockaddr_in6 *)addr)->sin6_addr)) == 1))
    {
        struct sockaddr_in6 *ipv6_addr;

        ipv6_addr            = (struct sockaddr_in6 *)addr;
        addr->ss_family      = AF_INET6;
        ipv6_addr->sin6_port = net_port;
        *addr_len            = sizeof(struct sockaddr_in6);
    }
    else
    {
        fprintf(stderr, "%s is not an IPv4 or an IPv6 address\n", address);
        *err = errno;
    }
}

int setSocketNonBlocking(int socket, int *err)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if(flags == -1)
    {
        *err = errno;
        return -1;
    }
    flags |= O_NONBLOCK;
    if(fcntl(socket, F_SETFL, flags) == -1)
    {
        *err = errno;
        return -1;
    }
    return 0;
}

static int setSockReuse(int fd, int *err)
{
    int opt;
    opt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        *err = errno;
        return -1;
    }
    return 0;
}

static int setup_tcp_server(const struct sockaddr_storage *addr, socklen_t addr_len, int backlog, int *err)
{
    int fd;
    int result;

    fd = socket(addr->ss_family, SOCK_STREAM, 0);    // NOLINT(android-cloexec-socket)

    if(fd == -1)
    {
        *err = errno;
        goto done;
    }

    result = setSocketNonBlocking(fd, err);
    if(result == -1)
    {
        goto done;
    }

    result = setSockReuse(fd, err);

    if(result == -1)
    {
        goto done;
    }

    result = bind(fd, (const struct sockaddr *)addr, addr_len);

    if(result == -1)
    {
        *err = errno;
        goto done;
    }

    result = listen(fd, backlog);

    if(result == -1)
    {
        *err = errno;
        goto done;
    }

done:
    return fd;
}

static int connect_to_server(struct sockaddr_storage *addr, socklen_t addr_len, int *err)
{
    int fd;
    int result;

    fd = socket(addr->ss_family, SOCK_STREAM, 0);    // NOLINT(android-cloexec-socket)

    if(fd == -1)
    {
        *err = errno;
        goto done;
    }

    result = connect(fd, (const struct sockaddr *)addr, addr_len);

    if(result == -1)
    {
        *err = errno;
        close(fd);
        fd = -1;
    }

done:
    return fd;
}

ssize_t convert_port(const char *str, in_port_t *port)
{
    char *endptr;
    long  val;

    val = strtol(str, &endptr, 10);    // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Check if no digits were found
    if(endptr == str)
    {
        return ERR_NO_DIGITS;
    }

    // Check for out-of-range errors
    if(val < 0 || val > UINT16_MAX)
    {
        return ERR_OUT_OF_RANGE;
    }

    // Check for trailing invalid characters
    if(*endptr != '\0')
    {
        return ERR_INVALID_CHARS;
    }

    *port = (in_port_t)val;
    return ERR_NONE;
}
