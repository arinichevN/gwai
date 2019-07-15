#include "../../dstructure_auto.h"
#include "../main.h"
#include "../../app.h"
#include "parallel.h"

static int serverm_initConnectionPool ( ServermConnList *list, int length ) {
    ALLOC_LIST ( list, length )
    if ( list->max_length != length ) {
        return 0;
    }
    int f=1;
    FORMLi {
        LIi.id = i;
        LIi.state = SERVERM_IDLE;
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

static int serverm_startConnectionThreads ( ServermConnList *list,  void * ( *thread_routine ) ( void * ) ) {
    FORLi {
        if ( !createMThread ( &LIi.thread, thread_routine, &LIi ) ) {
            putsde ( "failed to create thread\n" );
            return 0;
        }
    }
    return 1;
}

int serverm_init ( int *fd, int port,  int conn_num, ServermConnList *list,  void * ( *thread_routine ) ( void * ) ) {
    if ( !acpp_initServer ( fd, port ) ) {
        putsde ( "failed to initialize server socket\n" );
        return 0;
    }
    if ( !serverm_initConnectionPool ( list, conn_num ) ) {
        putsde ( "failed to initialize connection service pool\n" );
        return 0;
    }
    if ( !serverm_startConnectionThreads ( list, thread_routine ) ) {
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
				LIi.fd_conn = fd_conn;
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
