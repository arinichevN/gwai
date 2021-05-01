#ifndef ACP_SERIAL_SERVER_PORT_H
#define ACP_SERIAL_SERVER_PORT_H

#include <dirent.h>
#include "../../../dstructure.h"
#include "../../../debug.h"
#include "../../../app.h"
#include "../../../serial.h"
#include "../../main.h"
#include "../main.h"
#include "../port_param.h"
#include "command_node.h"


#define ACPSSP_CYCLE_DURATION_RUN			(struct timespec) {0, 70000UL}
#define ACPSSP_CYCLE_DURATION_TRY_OPEN		(struct timespec) {0, 700000000UL}

typedef struct acpsserialport_st AcpssPort;
struct acpsserialport_st {
	int fd;
	char buf[ACP_BUF_MAX_LENGTH];
	Thread thread;
	Mutex mutex;
	AcpSerialPortParam param;
	AcpssCommandNode *acnodes;
	size_t acnodes_count;
	struct timespec cycle_duration;
	void (*control)(AcpssPort *);
	AcpssPort *next;
};

extern AcpssPort *acpssp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config, AcpssCommandNode *acnodes, size_t acnodes_count);

extern void acpssp_free(AcpssPort *self);

#endif 
