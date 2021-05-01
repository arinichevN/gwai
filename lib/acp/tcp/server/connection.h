#ifndef ACP_TCP_SERVER_CONNECTION_H
#define ACP_TCP_SERVER_CONNECTION_H

#include "../../../timef.h"
#include "../../../dstructure_auto.h"
#include "../../../app.h"
#include "../main.h"
#include "serve_function.h"

typedef struct acpts_conn_st AcptsConnection;
struct acpts_conn_st{
	size_t id;
	int fd;
	AcptsServeFunction serve_function;
	void (*control)(AcptsConnection *);
	Thread thread;
	Mutex mutex;
	AcptsConnection *next;
};

extern AcptsConnection *acptsconn_new(int id, AcptsServeFunction serve_function);

extern const char *acptsconn_getStateStr(AcptsConnection *self);

extern int acptsconn_wakeUp(AcptsConnection *self, int fd);

extern void acptsconn_free(AcptsConnection *self);

#endif
