#ifndef ACP_SERIAL_CLIENT_PORT_H
#define ACP_SERIAL_CLIENT_PORT_H

#include "../../../debug.h"
#include "../../../dstructure.h"
#include "../../../app.h"
#include "../../../timef.h"
#include "../../../serial.h"
#include "../../main.h"
#include "../../command/main.h"
#include "../main.h"
#include "../port_param.h"
#include "id_list.h"

#define ACPSCP_CYCLE_DURATION_SEARCH		(struct timespec) {0UL, 100000000UL}
#define ACPSCP_CYCLE_DURATION_RUN			(struct timespec) {1UL, 0UL}
#define ACPSCP_CYCLE_DURATION_IDLE			(struct timespec) {1UL, 0UL}
#define ACPSCP_CYCLE_DURATION_TRY_OPEN		(struct timespec) {0UL, 700000000UL}
#define ACPSCP_CYCLE_DURATION_ID_EXISTS		(struct timespec) {0UL, 0UL}
#define ACPSCP_WAIT_IDLE_INTERVAL			(struct timespec) {3UL, 0UL}

typedef struct acpscp_st AcpscPort;
struct acpscp_st {
	int fd;
	AcpscIDLListm *ids;
	Ton jump_to_idle_tmr;
	AcpSerialPortParam param;
	Thread thread;
	Mutex mutex;
	struct timespec cycle_duration;
	void (*control)(AcpscPort *);
	AcpscPort *next;
};

extern AcpscPort *acpscp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config, AcpscIDLListm *ids);
extern void acpscp_free(AcpscPort *self);
extern void acpscp_lock(AcpscPort *self);
extern void acpscp_unlock(AcpscPort *self);
extern int acpscp_send(AcpscPort *self, const char *request_str);
extern int acpscp_readResponse(AcpscPort *self, char *buf, size_t buf_len);

#endif 
