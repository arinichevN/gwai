#include "../main.h"
#include "../../app.h"
#include "parallel.h"

const char *serverm_getConnStateStr(ServermConn *item){
	switch(item->state){
		case SERVERM_IDLE:return "idle";
		case SERVERM_BUSY:return "busy";
	}	
	return "?";
}

static int serverm_wakeUpConnection(ServermConn *item, int fd){
	if ( tryLockMutex ( &item->mutex ) ) {//printf("   locked id=%d\n", LIi.id);
		if ( item->state == SERVERM_IDLE ) {
			item->fd = fd;
			printdo ( "SERVER THREAD %zu MARKED TO START\n", item->id );
			item->state = SERVERM_BUSY;
			unlockMutex ( &item->mutex );
			return 1;
		}
		unlockMutex ( &item->mutex );
	} else {
		printdo ( "failed to lock mutex %zu\n", item->id );
	}
	return 0;
}


void *serverm_connThreadFunction ( void *arg ) {
    ServermConn *connection = arg;
    //printf ( "hello from server connection service thread %d\n", connection->id );
    //fflush ( stdout );
    while ( 1 ) {
        switch ( connection->state ) {
        case SERVERM_IDLE:
            NANOSLEEP(0,1000000);
            break;
        case SERVERM_BUSY:{
			printdo("SERVERM_BUSY %zu\n", connection->id);
         //   puts ( "\n\n" );
         //   printf ( "connection is busy %d\n", connection->id );
	        int old_state;
	        if ( threadCancelDisable ( &old_state ) ) {
	            lockMutex ( &connection->mutex ) ;
	            int fd = connection->fd;
	            while(1){
		            char buf[ACP_BUF_MAX_LENGTH];
		            if ( !acptcp_readPack(fd, buf, ACP_BUF_MAX_LENGTH)) {
						SERVER_GOTO_STOP
					}
		            if(!connection->serve_function(fd, buf)){
						SERVER_GOTO_STOP
					}
				}
	            SERVERM_STOP
	            threadSetCancelState ( old_state );
	        }
            break;}
        default:
            pthread_exit ( NULL );
            break;
        }
    }
}

static int serverm_startNewConnection(Serverm *item){
	ServermConnLList *list = &item->connection_list;
	if(list->length >= item->conn_num_max){
		printde ( "list->length = %zu, conn_num_max = %zu\n", list->length, item->conn_num_max  );
		return 0;
	}
	ServermConn *nconn = malloc(sizeof (ServermConn));
	if(nconn != NULL){
		nconn->id = list->length;
		nconn->state = SERVERM_IDLE;
		nconn->serve_function = item->serve_function;
		nconn->next = NULL;
		if ( !initMutex ( &nconn->mutex ) ) {
            putsde ( "failed to initialize mutex" );
            free(nconn);
            return 0;
        }
        if ( !createMThread ( &nconn->thread, &serverm_connThreadFunction, nconn ) ) {
            putsde ( "failed to create thread\n" );
            freeMutex ( &nconn->mutex );
            free(nconn);
            return 0;
        }
        size_t ol = list->length;
        LLIST_ADD_ITEM(nconn, list)
        if((ol + 1) != list->length){
			putsde ( "failed to add new connection to list\n" );
			STOP_THREAD(nconn->thread)
			free(nconn);
			return 0;
		}
		printdo("TCP CONNECTION SERVICE THREAD STARTED %zu\n", nconn->id);
        return 1;
	}
	putsde ( "failed to allocate memory\n" );
	return 0;
}

static void serverm_accept ( int fd, Serverm *item ) {
    int fd_conn = accept ( fd, ( struct sockaddr* ) NULL ,NULL );
	ServermConnLList *list = &item->connection_list;
	FOREACH_LLIST(curr,list,ServermConn) {
		if(serverm_wakeUpConnection(curr, fd_conn)){
			return;
		}
	}
	putsdo ( "serverm_accept: no free service threads, trying to create new service thread\n");
	if(!serverm_startNewConnection(item)){
		putsdo ( "serverm_accept: connection rejected\n");
		close(fd_conn);
	}
    
}


void *serverm_threadFunction ( void *arg ) {
    Serverm *item = arg;
    printf ( "hello from parallel server accept thread\n" );
    fflush ( stdout );
    while ( 1 ) {
		serverm_accept ( item->fd, item );
    }
}


static void serverm_freeConnection ( ServermConn *item ) {
    freeMutex ( &item->mutex );
    STOP_THREAD(item->thread)
    free ( item );
}

static void serverm_freeConnectionList ( ServermConnLList *list ) {
    ServermConn *item = list->top;
    while ( item != NULL ) {
        ServermConn *temp = item;
        item = item->next;
        serverm_freeConnection ( temp );
    }
    list->top = NULL;
    list->last = NULL;
    list->length = 0;
}

void serverm_free ( Serverm *item ) {
    STOP_THREAD(item->thread)
    close ( item->fd );
    serverm_freeConnectionList(&item->connection_list);
}

int serverm_init ( Serverm *item, int port,  size_t conn_num_max, int ( *serve_function ) ( int, char * ) ) {
	if(conn_num_max < 1){
		putsde ( "no connections required\n" );
		return 0;
	}
    item->fd = -1;
    item->conn_num_max = conn_num_max;
    item->serve_function = serve_function;
    if ( !acptcp_initServer ( &item->fd, port ) ) {
        putsde ( "failed to initialize server socket\n" );
        return 0;
    }
    LLIST_RESET(&item->connection_list);
    if(!serverm_startNewConnection(item)){
		putsde ( "failed to start connection service thread\n" );
        return 0;
	}
    if ( !createMThread ( &item->thread, serverm_threadFunction, item ) ) {
		putsde ( "failed to create server main thread\n" );
		return 0;
    }
    return 1;
}
