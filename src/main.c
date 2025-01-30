#include "../include/database.h"
#include "messaging.h"
#include "networking.h"
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <memory.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define UNKNOWN_OPTION_MESSAGE_LEN 22
#define BUFLEN 1024

typedef struct
{
    char     *addr;
    in_port_t port;
} Arguments;

_Noreturn void usage(const char *binary_name, int exit_code, const char *message);
void           get_arguments(Arguments *args, int argc, char *argv[]);
void           validate_arguments(const char *binary_name, const Arguments *args);

int main(int argc, char *argv[])
{
    pid_t pid;

    int sockfd;

    Arguments args;

    ssize_t result;
    int     err;

    memset(&args, 0, sizeof(Arguments));
    get_arguments(&args, argc, argv);
    validate_arguments(argv[0], &args);

    printf("Listening on %s:%d\n", args.addr, args.port);

    // Start TCP Server
    sockfd = tcp_socket(args.addr, args.port);
    if(sockfd < 0)
    {
        fprintf(stderr, "main::tcp_socket: Failed to create TCP socket.\n");
        return EXIT_FAILURE;
    }

    result = database_connect(&err);
    printf("result: %d\n", (int)result);

    if(result == -1)
    {
        perror("database error");
    }

    // Wait for client connections
    pid = 1;
    while(pid > 0)
    {
        int                connfd;
        struct sockaddr_in connaddr;
        socklen_t          connsize;

        // !!BLOCKING!! Get client connection
        connsize = sizeof(struct sockaddr_in);
        memset(&connaddr, 0, connsize);

        errno  = 0;
        connfd = accept(sockfd, (struct sockaddr *)&connaddr, &connsize);
        if(connfd < 0)
        {
            perror("main::accept");
            continue;
        }

        printf("New connection from: %s:%d\n", inet_ntoa(connaddr.sin_addr), connaddr.sin_port);

        // Fork the process
        errno = 0;
        pid   = fork();
        if(pid < 0)
        {
            perror("main::fork");
            continue;
        }

        if(pid == 0)
        {
            close(sockfd);

            err = 0;
            if(copy(connfd, connfd, BUFLEN, &err) < 0)
            {
                errno = err;
                perror("main::copy");
            }
        }

        close(connfd);
    }

    return EXIT_SUCCESS;
}

_Noreturn void usage(const char *binary_name, int exit_code, const char *message)
{
    if(message)
    {
        fprintf(stderr, "%s\n\n", message);
    }

    fprintf(stderr, "Usage: %s [-h] -a <address> -p <port>\n", binary_name);
    fputs("Options:\n", stderr);
    fputs("  -h, --help                         Display this help message\n", stderr);
    fputs("  -a <address>, --address <address>  The address of remote server.\n", stderr);
    fputs("  -p <port>,    --port <port>        The server port to use.\n", stderr);
    exit(exit_code);
}

void get_arguments(Arguments *args, int argc, char *argv[])
{
    int opt;
    int err;

    static struct option long_options[] = {
        {"address", required_argument, NULL, 'a'},
        {"port",    required_argument, NULL, 'p'},
        {"help",    no_argument,       NULL, 'h'},
        {NULL,      0,                 NULL, 0  }
    };

    while((opt = getopt_long(argc, argv, "ha:p:", long_options, NULL)) != -1)
    {
        switch(opt)
        {
            case 'a':
                args->addr = optarg;
                break;
            case 'p':
                args->port = convert_port(optarg, &err);

                if(err != 0)
                {
                    usage(argv[0], EXIT_FAILURE, "Port must be between 1 and 65535");
                }
                break;
            case 'h':
                usage(argv[0], EXIT_SUCCESS, NULL);
            case '?':
                if(optopt != 'a' && optopt != 'p')
                {
                    char message[UNKNOWN_OPTION_MESSAGE_LEN];

                    snprintf(message, sizeof(message), "Unknown option '-%c'.", optopt);
                    usage(argv[0], EXIT_FAILURE, message);
                }
                break;
            default:
                usage(argv[0], EXIT_FAILURE, NULL);
        }
    }
}

void validate_arguments(const char *binary_name, const Arguments *args)
{
    if(args->addr == NULL)
    {
        usage(binary_name, EXIT_FAILURE, "You must provide an ipv4 address to connect to.");
    }

    if(args->port == 0)
    {
        usage(binary_name, EXIT_FAILURE, "You must provide an available port to connect to.");
    }
}
