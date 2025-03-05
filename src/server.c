#include "args.h"
#include "fsm.h"
#include "messaging.h"
#include "networking.h"
#include "utils.h"
#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define INADDRESS "0.0.0.0"
#define OUTADDRESS "127.0.0.1"
#define BACKLOG 5
#define PORT "8081"
#define SM_PORT "8082"

int main(int argc, char *argv[])
{
    int    retval;
    int    server_fd;
    int    sm_fd;
    args_t args;
    int    err;

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

    setup_signal();

    printf("Server launching... (press Ctrl+C to interrupt)\n");

    retval = EXIT_SUCCESS;

    memset(&args, 0, sizeof(args_t));
    args.addr = INADDRESS;
    convert_port(PORT, &args.port);
    args.sm_addr = OUTADDRESS;
    convert_port(SM_PORT, &args.sm_port);

    get_arguments(&args, argc, argv);

    // Start TCP Server
    server_fd = tcp_server(args.addr, args.port, BACKLOG, &err);
    if(server_fd < 0)
    {
        fprintf(stderr, "main::tcp_server: Failed to create TCP server.\n");
        return EXIT_FAILURE;
    }

    printf("Listening on %s:%d\n", args.addr, args.port);

    // Start TCP Client
    sm_fd = tcp_client(args.sm_addr, args.sm_port, &err);
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
    err = 0;
    event_loop(server_fd, &err);

    close(sm_fd);
    close(server_fd);
    return retval;
}
