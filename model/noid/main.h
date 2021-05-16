#ifndef MODEL_NOID_H
#define MODEL_NOID_H

#include "../../lib/debug.h"
#include "../../lib/app.h"
#include "../../lib/timef.h"
#include "command/get/interval/main.h"

#define NOID_CYCLE_DURATION (struct timespec){0UL, 10000000UL}

typedef struct noid_st Noid;
struct noid_st {
	int id;
	NoidIntervalGetCommandList igcommands;
	struct timespec cycle_duration;
	int (*control)(Noid *);
	Mutex mutex;
	Thread thread;
};

#define noid_control(ITEM, FD) (ITEM)->control(ITEM, FD)
extern void noid_resetData(Noid *self);
extern NoidGetCommand *noid_getIntervalGetCmd(Noid *self, int cmd);
extern void noid_connectToSerialPort(Noid *self, Mutex *serial_port_mutex, int serial_port_fd);
extern void noid_start(Noid *self);
extern void noid_terminate(Noid *self);
extern void noid_free(Noid *self);
extern int noid_begin(Noid *self);
extern int noid_setParam(Noid *self, int id, const char *iget_file, const char *iget_dir, const char *file_type);
extern const char *noid_getStateStr(Noid *self);

#endif 
