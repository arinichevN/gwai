#ifndef MODEL_SLAVE_INTERVAL_GET_COMMAND_H
#define MODEL_SLAVE_INTERVAL_GET_COMMAND_H

#include "../../lib/ACP/command/util.h"
#include "../../lib/ACP/TCP/ACPTCP.h"
#include "../../lib/ACP/serial/ACPSerial.h"
#include "../../lib/ACP/serial/client/ACPSC.h"
#include "../../lib/app.h"
#include "../../lib/tsv.h"
#include "../SlaveGetCommand/main.h"

typedef struct sigc_st SlaveIntervalGetCommand;
struct sigc_st{
	SlaveGetCommand command;
	struct timespec interval;
	Ton tmr;
	int (*control)(SlaveIntervalGetCommand *, int);
};

DEC_LIST(SlaveIntervalGetCommand)

extern void sigc_free(SlaveIntervalGetCommand *self);
extern void sigc_reset(SlaveIntervalGetCommand *self);
extern int sigc_control(SlaveIntervalGetCommand *self, int fd, Mutex *smutex, int channel_id);
extern int sigcList_init(SlaveIntervalGetCommandList *list, const char *dir, const char *file_name, const char *file_type);
extern const char *sigc_getStateStr(SlaveIntervalGetCommand *self);
#define sigc_control(self, remote_id) (self)->control(self, remote_id)

#endif 
