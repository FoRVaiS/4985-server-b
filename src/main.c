#include "args.h"
#include "messaging.h"
#include "networking.h"
#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__linux__) && defined(__clang__)
_Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")
#endif

#define INADDRESS "0.0.0.0"
#define OUTADDRESS "127.0.0.1"
#define PORT "8081"
#define SM_PORT "8082"
#define SIG_BUF 50

    static volatile sig_atomic_t running;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,-warnings-as-errors)

static const char *const ending = "\nShutting down gracefully...\n";

static void handle_signal(int sig)
{
    char message[SIG_BUF];

    snprintf(message, sizeof(message), "Caught signal: %d (%s)\n", sig, strsignal(sig));
    write(STDOUT_FILENO, message, strlen(message));

    if(sig == SIGINT)
    {
        running = 0;
        snprintf(message, sizeof(message), "%s\n", ending);
    }
    write(STDOUT_FILENO, message, strlen(message));
}

int main(int argc, char *argv[])
{
    int retval;

    struct sigaction    sa;
    pid_t               pid;
    int                 sockfd;
    int                 sm_fd;
    Arguments           args;
    int                 err;
    const unsigned char sm_msg[] = {
        ACC_Login,    // 10
        0x01,         // 1
        0x00,         // 0
        0x04,         // 4
        0x02,         // 2
        0x02,         // 2
        0x00,         // 0
        ACC_Login,    // 10
    };

    sa.sa_handler = handle_signal;    // Set handler function for SIGINT
    sigemptyset(&sa.sa_mask);         // Don't block any additional signals
    sa.sa_flags = 0;

    // Register signal handler
    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Server launching... (press Ctrl+C to interrupt)\n");

    memset(&args, 0, sizeof(Arguments));
    args.addr    = INADDRESS;
    args.port    = convert_port(PORT, &err);
    args.sm_addr = OUTADDRESS;
    args.sm_port = convert_port(SM_PORT, &err);

    get_arguments(&args, argc, argv);
    validate_arguments(argv[0], &args);

    retval = EXIT_SUCCESS;

    // Start TCP Server
    sockfd = tcp_server(&args);
    if(sockfd < 0)
    {
        fprintf(stderr, "main::tcp_server: Failed to create TCP server.\n");
        return EXIT_FAILURE;
    }

    printf("Listening on %s:%d\n", args.addr, args.port);

    // Start TCP Client
    sm_fd = tcp_client(&args);
    // if(sm_fd < 0)
    // {
    //     fprintf(stderr, "main::tcp_server: Failed to create TCP server.\n");
    //     return EXIT_FAILURE;
    // }

    printf("Connect to server manager at %s:%d\n", args.sm_addr, args.sm_port);

    // just for demo
    if(write(sm_fd, sm_msg, sizeof(user_count_t)) < 0)
    {
        fprintf(stderr, "main::tcp_client: write to server manager failed.\n");
        // return EXIT_FAILURE;
    }

    // Wait for client connections
    err     = 0;
    running = 1;
    while(running)
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
            // perror("main::accept");
            continue;
        }

        printf("New connection from: %s:%d\n", inet_ntoa(connaddr.sin_addr), connaddr.sin_port);

        // Fork the process
        errno = 0;
        pid   = fork();
        if(pid < 0)
        {
            perror("main::fork");
            close(connfd);
            continue;
        }

        if(pid == 0)
        {
            retval = (int)request_handler(connfd);
            close(connfd);
            goto exit;
        }

        close(connfd);
    }

exit:
    close(sm_fd);
    close(sockfd);
    return retval;
}
