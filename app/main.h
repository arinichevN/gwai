#ifndef APP_H
#define APP_H

#include "../lib/debug.h"
#include "../lib/app.h"

#include "../lib/ACP/TCP/server/ACPTS.h"
#include "../lib/ACP/serial/client/ACPSC.h"
#include "../model/Channel/list.h"
#include "tcp_server.h"
#include "common.h"



extern void (*app_control)();
extern void app_begin();
extern void app_reset();
extern const char *app_getStateStr();
extern const char *app_getErrorStr();

#endif




