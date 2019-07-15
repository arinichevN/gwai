
#ifndef SRT_H
#define SRT_H

#include "../../lib/app.h"
#include "../../lib/timef.h"
#include "../../lib/serial.h"
#include "../../lib/puart.h"

#define APP_NAME srt
#define APP_NAME_STR TOSTRING(APP_NAME)

extern int readSettings();

extern void appRun(int *state, int init_state);

extern void *threadFunction(void *arg);

extern int initApp();

extern int initData();

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif

