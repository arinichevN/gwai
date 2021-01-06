#ifndef ACP_SERIAL_SERVER_PORT_H
#define ACP_SERIAL_SERVER_PORT_H

#include <dirent.h>
#include "../../../dstructure.h"
#include "../../../debug.h"
#include "../../../app.h"
#include "../../../serial.h"
#include "../../ACP.h"
#include "../ACPSerial.h"
#include "../ACPSerialPortParam.h"
#include "ACPSSCommandNode.h"


#define ACPSSP_CYCLE_DURATION_RUN			(struct timespec) {0, 70000UL}
#define ACPSSP_CYCLE_DURATION_TRY_OPEN		(struct timespec) {0, 700000000UL}

typedef struct acpsserialport_st ACPSSPort;
struct acpsserialport_st {
	int fd;
	char buf[ACP_BUF_MAX_LENGTH];
	Thread thread;
	Mutex mutex;
	ACPSerialPortParam param;
	ACPSSCommandNode *acnodes;
	size_t acnodes_count;
	struct timespec cycle_duration;
	void (*control)(ACPSSPort *);
	ACPSSPort *next;
};

extern ACPSSPort *acpssp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config, ACPSSCommandNode *acnodes, size_t acnodes_count);

extern void acpssp_free(ACPSSPort *self);

#endif 
