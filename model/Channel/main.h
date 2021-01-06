#ifndef MODEL_CHANNEL_H
#define MODEL_CHANNEL_H

#include "../../lib/debug.h"
#include "../../lib/app.h"
#include "../../lib/timef.h"
#include "../SlaveIntervalGetCommand/main.h"

#define CHANNEL_CYCLE_DURATION (struct timespec){0, 10000000}

typedef struct channel_st Channel;
struct channel_st {
	int id;
	SlaveIntervalGetCommandList igcmd_list;
	struct timespec cycle_duration;
	int (*control)(Channel *);
	Mutex mutex;
	Thread thread;
};

#define channel_control(ITEM, FD) (ITEM)->control(ITEM, FD)
extern void channel_resetData(Channel *item);
extern SlaveGetCommand *channel_getIntervalGetCmd(Channel *channel, int cmd);
extern void channel_connectToSerialPort(Channel *self, Mutex *serial_port_mutex, int serial_port_fd);
extern void channel_start(Channel *item);
extern void channel_free(Channel *self);
extern int channel_begin(Channel *self);
extern int channel_setParam(Channel *self, int id, const char *iget_file, const char *iget_dir, const char *file_type);
extern const char *channel_getStateStr(Channel *item);
extern int channel_slaveToClient (Channel *channel, char *pack_str, int tcp_fd);
extern int channel_slaveToClientText (Channel *channel, char *pack_str, int tcp_fd);
extern int channel_sendRawDataToSlave (Channel *channel, char *pack_str);

#endif 
