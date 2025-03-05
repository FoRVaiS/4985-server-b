#ifndef UTILS_H
#define UTILS_H

#include <signal.h>

extern volatile sig_atomic_t running;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,-warnings-as-errors)

void nfree(void **ptr);

void setup_signal(void);

#endif
