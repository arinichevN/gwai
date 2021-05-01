#ifndef APP_TCP_SERVER_H
#define APP_TCP_SERVER_H

#include "../lib/debug.h"
#include "../lib/app.h"
#include "../lib/controller.h"
#include "../lib/acp/serial/client/main.h"
#include "../lib/acp/tcp/main.h"
#include "../model/noid/list.h"
#include "common.h"
#include "print.h"

extern int serveTCPRequest(int tcp_fd, const char *tcp_request_str);

#endif 
