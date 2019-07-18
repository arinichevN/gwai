#include "../../dstructure_auto.h"
#include "../main.h"
#include "../../app.h"
#include "parallel.h"

void *serverm_threadFunction ( void *arg ) {
    ServermConn *connection = arg;
    printf ( "hello from server thread %d\n", connection->id );
    fflush ( stdout );
    while ( 1 ) {
        switch ( connection->state ) {
        case SERVERM_IDLE:
            nanosleep ( &(struct timespec) {0,10000000}, NULL );
            break;
        case SERVERM_BUSY:{
			//printdo("SERVERM_BUSY %d\n", connection->id);
         //   puts ( "\n\n" );
         //   printf ( "connection is busy %d\n", connection->id );
            lockMutex ( &connection->mutex ) ;
            int SERVER_FD = connection->fd;
            SERVER_READ_CMD
            //printf("command: %s\n", cmd);
            connection->serveFunction(SERVER_FD, SERVER_CMD);
            SERVERM_STOP
            break;}
        default:
            pthread_exit ( NULL );
            break;
        }
    }

}

static int serverm_initConnectionPool ( ServermConnList *list, void ( *serveFunction ) ( int, const char * ), int length ) {
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
        if ( !createMThread ( &LIi.thread, serverm_threadFunction, &LIi ) ) {
            putsde ( "failed to create thread\n" );
            return 0;
        }
    }
    return 1;
}

int serverm_init ( int *fd, int port,  int conn_num, ServermConnList *list,  void ( *serveFunction ) ( int, const char * ) ) {
    if ( !acpp_initServer ( fd, port ) ) {
        putsde ( "failed to initialize server socket\n" );
        return 0;
    }
    if ( !serverm_initConnectionPool ( list, serveFunction, conn_num ) ) {
        putsde ( "failed to initialize connection service pool\n" );
        return 0;
    }
    if ( !serverm_startConnectionThreads ( list ) ) {
        putsde ( "failed to start connection threads\n" );
        return 0;
    }
    return 1;
}

void serverm_accept ( int fd, ServermConnList *list ) {
    int fd_conn = accept ( fd, ( struct sockaddr* ) NULL ,NULL );
    FORLi {
		printf("we have id=%d\n", LIi.id);
	}
	FORLi {
		printf("try lock id=%d\n", LIi.id);
		if ( tryLockMutex ( &LIi.mutex ) ) {printf("   locked id=%d\n", LIi.id);
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

void serverm_free ( int fd, ServermConnList *list ) {
    close ( fd );
    FORLi {
        pthread_cancel ( LIi.thread );
        pthread_join(LIi.thread, NULL);
    }
    FREE_LIST ( list )
}
