#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "messaging.h"

ssize_t packet_handler(const request_t *request, response_t *response);

ssize_t account_create(request_t *request, response_t *response);

ssize_t account_login(const request_t *request, response_t *response);

ssize_t account_logout(request_t *request, response_t *response);

#endif    // ACCOUNT_H
