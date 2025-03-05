#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "messaging.h"

extern const funcMapping acc_func[];

ssize_t account_create(request_t *request);

ssize_t account_login(request_t *request);

ssize_t account_logout(request_t *request);

ssize_t account_edit(request_t *request);

#endif    // ACCOUNT_H
