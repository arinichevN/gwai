#ifndef ACP_TCP_SERVER_CONNECTION_H
#define ACP_TCP_SERVER_CONNECTION_H

#include "../ACPTCP.h"
#include "../../../timef.h"
#include "../../../dstructure_auto.h"
#include "../../../app.h"
#include "ACPTSServeFunction.h"

typedef struct acpts_conn_st ACPTSConnection;
struct acpts_conn_st{
	size_t id;
	int fd;
	ACPTSServeFunction serve_function;
	void (*control)(ACPTSConnection *);
	Thread thread;
	Mutex mutex;
	ACPTSConnection *next;
};

extern ACPTSConnection *acptsconn_new(int id, ACPTSServeFunction serve_function);

extern const char *acptsconn_getStateStr(ACPTSConnection *self);

extern int acptsconn_wakeUp(ACPTSConnection *self, int fd);

extern void acptsconn_free(ACPTSConnection *self);

#endif
