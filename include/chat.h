#ifndef CHAT_H
#define CHAT_H

#include "messaging.h"

extern const funcMapping chat_func[];

ssize_t chat_broadcast(request_t *request);

#endif    // CHAT_H
