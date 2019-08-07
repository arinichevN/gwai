#include "common.h"
#include "../../timef.h"
#define SERVERM_STOP server_stop:close(connection->fd);connection->state=SERVERM_IDLE;unlockMutex(&connection->mutex);


typedef struct {
	int id;
	int state;
	int fd;
	void ( *serveFunction ) ( int, const char * );
	pthread_t thread;
	Mutex mutex;
}ServermConn;
DEC_LIST(ServermConn)

typedef struct {
	ServermConnList connection_list;
	int fd;
	pthread_t thread;
} Serverm;

enum serverm_state{
	SERVERM_IDLE,
	SERVERM_BUSY
};

extern int serverm_init(Serverm *item, int port,  int conn_num, void ( *serveFunc ) ( int, const char * )) ;

extern void serverm_free ( Serverm *item );
