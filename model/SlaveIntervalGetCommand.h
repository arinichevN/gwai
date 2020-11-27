#ifndef MODEL_SLAVE_INTERVAL_GET_COMMAND_H
#define MODEL_SLAVE_INTERVAL_GET_COMMAND_H

#include "../lib/acp/tcp/main.h"
#include "../lib/acp/serial/main.h"
#include "../lib/app.h"
#include "../lib/tsv.h"
#include "SlaveGetCommand.h"

typedef struct sigc_st SlaveIntervalGetCommand;
struct sigc_st{
    SlaveGetCommand command;
    struct timespec interval;
    Ton tmr;
    int (*control)(SlaveIntervalGetCommand *, int, Mutex *, int);
};

DEC_LIST(SlaveIntervalGetCommand)

extern void sigc_free(SlaveIntervalGetCommand *item);
extern void sigc_reset(SlaveIntervalGetCommand *item);
extern int sigc_control(SlaveIntervalGetCommand *item, int fd, Mutex *smutex, int channel_id );
extern int sigcList_init(SlaveIntervalGetCommandList *list, const char *dir, const char *file_name, const char *file_type);
extern const char *sigc_getStateStr(SlaveIntervalGetCommand *item);
#define sigc_control(item, fd, smutex, channel_id ) (item)->control(item, fd, smutex, channel_id)

#endif 
