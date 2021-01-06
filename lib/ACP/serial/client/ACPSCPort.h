#ifndef ACP_SERIAL_CLIENT_PORT_H
#define ACP_SERIAL_CLIENT_PORT_H

#include "../../../debug.h"
#include "../../../dstructure.h"
#include "../../../app.h"
#include "../../../timef.h"
#include "../../../serial.h"
#include "../../ACP.h"
#include "../../command/main.h"
#include "../ACPSerial.h"
#include "../ACPSerialPortParam.h"

#define ACPSCP_CYCLE_DURATION_RUN			(struct timespec) {0, 500000000UL}
#define ACPSCP_CYCLE_DURATION_TRY_OPEN		(struct timespec) {0, 700000000UL}
#define ACPSCP_CYCLE_DURATION_ID_EXISTS		(struct timespec) {0, 0}

typedef struct acpscp_st ACPSCPort;
struct acpscp_st {
	int fd;
	int remote_id;
	void *mapper;
	void (*onCheckRemoteIDExistsDone)(void *, int, int, void *);
	void (*onConnected)(void *, void *);
	void (*onDisconnected)(void *, void *);
	ACPSerialPortParam param;
	Thread thread;
	Mutex mutex;
	struct timespec cycle_duration;
	void (*control)(ACPSCPort *);
	ACPSCPort *next;
};

extern ACPSCPort *acpscp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config);
extern void acpscp_free(ACPSCPort *self);
extern void acpscp_lock(ACPSCPort *self);
extern void acpscp_unlock(ACPSCPort *self);
extern int acpscp_send(ACPSCPort *self, const char *request_str);
extern int acpscp_readResponse(ACPSCPort *self, char *buf, size_t buf_len);
extern void acpscp_checkRemoteIDExists(ACPSCPort *self, int id);

#endif 
