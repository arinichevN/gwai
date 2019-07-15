

#define SERVERM_STOP server_stop:close(connection->fd_conn);connection->state=SERVERM_IDLE;unlockMutex(&connection->mutex);


typedef struct {
	int id;
	int state;
	int fd_conn;
	pthread_t thread;
	Mutex mutex;
}ServermConn;
DEC_LIST(ServermConn)

enum serverm_state{
	SERVERM_IDLE,
	SERVERM_BUSY
};

extern int serverm_init(int *fd, int port,  int conn_num, ServermConnList *list,  void * ( *thread_routine ) ( void * )) ;

extern void serverm_accept ( int fd, ServermConnList *list );

extern void serverm_free ( int fd, ServermConnList *list );
