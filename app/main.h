#ifndef APP_H
#define APP_H

#include "../lib/debug.h"
#include "../lib/app.h"

#include "../lib/acp/tcp/server/main.h"
#include "../lib/acp/serial/client/main.h"
#include "../model/noid/list.h"
#include "tcp_server.h"
#include "common.h"

extern void (*app_control)();
extern void app_begin();
extern void app_reset();
extern const char *app_getStateStr();
extern const char *app_getErrorStr();

#endif




