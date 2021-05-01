#ifndef MODEL_NOID_COMMAND_INTERVAL_GET_H
#define MODEL_NOID_COMMAND_INTERVAL_GET_H

#include "../../../../../lib/acp/command/util.h"
#include "../../../../../lib/acp/tcp/main.h"
#include "../../../../../lib/acp/serial/main.h"
#include "../../../../../lib/acp/serial/client/main.h"
#include "../../../../../lib/app.h"
#include "../../../../../lib/tsv.h"
#include "../main.h"

typedef struct nigc_st NoidIntervalGetCommand;
struct nigc_st{
	NoidGetCommand command;
	struct timespec interval;
	Ton tmr;
	int (*control)(NoidIntervalGetCommand *, int);
};

DEC_LIST(NoidIntervalGetCommand)

extern void nigc_free(NoidIntervalGetCommand *self);
extern void nigc_reset(NoidIntervalGetCommand *self);
extern int nigc_control(NoidIntervalGetCommand *self, int fd, Mutex *smutex, int remote_id);
extern int nigcList_init(NoidIntervalGetCommandList *list, const char *dir, const char *file_name, const char *file_type);
extern const char *nigc_getStateStr(NoidIntervalGetCommand *self);
#define nigc_control(self, remote_id) (self)->control(self, remote_id)

#endif 
