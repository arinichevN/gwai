#ifndef APP_TCP_SERVER_H
#define APP_TCP_SERVER_H

#include "../lib/debug.h"
#include "../lib/app.h"
#include "../lib/controller.h"
#include "../lib/ACP/serial/client/ACPSC.h"
#include "../lib/ACP/TCP/ACPTCP.h"
#include "../model/Channel/list.h"
#include "common.h"
#include "print.h"

extern int serveTCPRequest(int tcp_fd, const char *tcp_request_str);

#endif 
