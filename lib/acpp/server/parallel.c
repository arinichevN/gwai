#include "../../dstructure_auto.h"
#include "../main.h"
#include "../../app.h"
#include "parallel.h"

static void serverm_accept ( int fd, ServermConnList *list ) {
    int fd_conn = accept ( fd, ( struct sockaddr* ) NULL ,NULL );
   // FORLi {
	//	printf("we have id=%d\n", LIi.id);
	//}
	FORLi {
		//printf("try lock id=%d\n", LIi.id);
		if ( tryLockMutex ( &LIi.mutex ) ) {//printf("   locked id=%d\n", LIi.id);
			if ( LIi.state == SERVERM_IDLE ) {
				LIi.fd = fd_conn;
				printdo ( "THREAD %d MARKED TO START\n", i );
				LIi.state = SERVERM_BUSY;
				unlockMutex ( &LIi.mutex );
				return;
			}
			unlockMutex ( &LIi.mutex );
		} else {
			printdo ( "failed to lock mutex %d\n",i );
		}
	}
	putsdo ( "serverm_accept: no free service threads, rejecting connection\n");
    close(fd_conn);
}

void *serverm_connThreadFunction ( void *arg ) {
    ServermConn *connection = arg;
    printf ( "hello from server connection service thread %d\n", connection->id );
    fflush ( stdout );
    while ( 1 ) {
        switch ( connection->state ) {
        case SERVERM_IDLE:
            NANOSLEEP(0,10000000);
            break;
        case SERVERM_BUSY:{
			//printdo("SERVERM_BUSY %d\n", connection->id);
         //   puts ( "\n\n" );
         //   printf ( "connection is busy %d\n", connection->id );
	        int old_state;
	        if ( threadCancelDisable ( &old_state ) ) {
	            lockMutex ( &connection->mutex ) ;
	            int SERVER_FD = connection->fd;
	            SERVER_READ_CMD
	            //printf("command: %s\n", cmd);
	            connection->serveFunction(SERVER_FD, SERVER_CMD);
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

void *serverm_threadFunction ( void *arg ) {
    Serverm *item = arg;
    printf ( "hello from parallel server accept thread\n" );
    fflush ( stdout );
    while ( 1 ) {
		serverm_accept ( item->fd, &item->connection_list );
    }
}

static int serverm_initConnectionPool ( ServermConnList *list, void ( *serveFunction ) ( int, const char * ), int length ) {
    RESET_LIST(list)
    ALLOC_LIST ( list, length )
    if ( list->max_length != length ) {
        return 0;
    }
    int f=1;
    FORMLi {
        LIi.id = i;
        LIi.state = SERVERM_IDLE;
        LIi.serveFunction = serveFunction;
        if ( !initMutex ( &LIi.mutex ) ) {
            f=0;
            putsde ( "failed to initialize mutex" );
            break;
        }
        list->length++;
    }
    if ( !f ) {
        FREE_LIST ( list )
        return 0;
    }
    return 1;
}

static int serverm_startConnectionThreads ( ServermConnList *list ) {
    FORLi {
        if ( !createMThread ( &LIi.thread, serverm_connThreadFunction, &LIi ) ) {
            putsde ( "failed to create thread\n" );
            return 0;
        }
    }
    return 1;
}

int serverm_init ( Serverm *item, int port,  int conn_num, void ( *serveFunction ) ( int, const char * ) ) {
    item->fd = -1;
    if ( !acpp_initServer ( &item->fd, port ) ) {
        putsde ( "failed to initialize server socket\n" );
        return 0;
    }
    if ( !serverm_initConnectionPool ( &item->connection_list, serveFunction, conn_num ) ) {
        putsde ( "failed to initialize connection service pool\n" );
        return 0;
    }
    if ( !serverm_startConnectionThreads ( &item->connection_list ) ) {
        putsde ( "failed to start connection threads\n" );
        return 0;
    }
    if ( !createMThread ( &item->thread, serverm_threadFunction, item ) ) {
		putsde ( "failed to create server main thread\n" );
		return 0;
    }
    return 1;
}



void serverm_free ( Serverm *item ) {
    while(pthread_cancel ( item->thread )!= 0){;}
    pthread_join(item->thread, NULL);
    close ( item->fd );
    FORLISTN(item->connection_list, i) {
        while(pthread_cancel ( item->connection_list.item[i].thread )!=0){;}
        pthread_join(item->connection_list.item[i].thread, NULL);
    }
    FREE_LIST ( &item->connection_list )
}
