#ifndef ACP_SERIAL_SPY_PORT_H
#define ACP_SERIAL_SPY_PORT_H

#include "../../../../debug.h"
#include "../../../../dstructure.h"
#include "../../../../app.h"
#include "../../../../timef.h"
#include "../../../../serial.h"
#include "../../../../noid.h"
#include "../../../main.h"
#include "../../../command/main.h"
#include "../../main.h"
#include "../../port_param.h"
#include "../id/list.h"

#define ACPSYP_CYCLE_DURATION_RUN			(struct timespec) {1UL, 0UL}
#define ACPSYP_CYCLE_DURATION_TRY_OPEN		(struct timespec) {0UL, 700000000UL}

typedef struct acpsyp_st AcpsyPort;
struct acpsyp_st {
	int fd;
	AcpsyIDList *ids;
	AcpSerialPortParam param;
	Thread thread;
	struct timespec cycle_duration;
	void (*control)(AcpsyPort *);
};

//extern AcpsyPort *acpsyp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDList *ids);

extern int acpsyp_begin(AcpsyPort *self, const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDList *ids);
extern void acpsyp_free(AcpsyPort *self);
extern void acpsyp_lock(AcpsyPort *self);
extern void acpsyp_unlock(AcpsyPort *self);
extern int acpsyp_send(AcpsyPort *self, const char *request_str);
extern int acpsyp_readResponse(AcpsyPort *self, char *buf, size_t buf_len);

#endif 
