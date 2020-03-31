#include "common.h"
#include "../../../timef.h"
#include "../../../dstructure_auto.h"
#include "../../../app.h"

#define SERVERM_STOP server_stop:close(connection->fd);connection->state=SERVERM_IDLE;unlockMutex(&connection->mutex);


struct serverm_conn_st{
	size_t id;
	int state;
	int fd;
	int ( *serve_function ) ( int, char * );
	pthread_t thread;
	Mutex mutex;
	struct serverm_conn_st *next;
};

typedef struct serverm_conn_st ServermConn;
DEC_LLIST(ServermConn)

typedef struct {
	ServermConnLList connection_list;
	size_t conn_num_max;
	int ( *serve_function ) ( int, char * );
	int fd;
	pthread_t thread;
} Serverm;

enum serverm_state{
	SERVERM_IDLE,
	SERVERM_BUSY
};

extern const char *serverm_getConnStateStr(ServermConn *item);

extern int serverm_init(Serverm *item, int port,  size_t conn_num_max, int ( *serveFunc ) ( int, char * )) ;

extern void serverm_free ( Serverm *item );
