#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__) && defined(__clang__)
_Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")
#endif

#define SIG_BUF 50

    volatile sig_atomic_t running = 1;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,-warnings-as-errors)

/* Calls `free()` and nullifies the ptr. */
void nfree(void **ptr)
{
    if(ptr != NULL && *ptr != NULL)
    {
        free(*ptr);
        *ptr = NULL;
    }
}

static void handle_signal(int sig)
{
    char message[SIG_BUF];

    snprintf(message, sizeof(message), "Caught signal: %d (%s)\n", sig, strsignal(sig));
    write(STDOUT_FILENO, message, strlen(message));

    if(sig == SIGINT)
    {
        running = 0;
        snprintf(message, sizeof(message), "\n%s\n", "Shutting down gracefully...");
    }
    write(STDOUT_FILENO, message, strlen(message));
}

void setup_signal(void)
{
    struct sigaction sa;

    sa.sa_handler = handle_signal;    // Set handler function for SIGINT
    sigemptyset(&sa.sa_mask);         // Don't block any additional signals
    sa.sa_flags = 0;

    // Register signal handler
    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
