#include "main.h"

static int app_state = APP_INIT;

static TSVresult config_tsv = TSVRESULT_INITIALIZER;
static char *db_prog_path;

static Mutex db_mutex = MUTEX_INITIALIZER;

static int sock_port = -1;
static int server_fd = -1;
static int conn_num = 0;
static int max_retry;
static ServermConnList sc_list = LIST_INITIALIZER;

static Mutex serial_thread_list_mutex = MUTEX_INITIALIZER;
static SerialThreadStarter serial_thread_starter;
static SerialThreadLList serial_thread_list = LLIST_INITIALIZER;
static ChannelList channel_list = LIST_INITIALIZER;

#include "util.c"
#include "channel.c"
#include "serialThread.c"
#include "serialThreadStarter.c"
#include "data.c"

int readSettings ( TSVresult* r, const char *data_path, int *port, struct timespec *cd, int *conn_num, int *max_retry, int *serial_rate, char **serial_config, char **serial_pattern, char **db_prog_path ) {
    if ( !TSVinit ( r, data_path ) ) {
        return 0;
    }
    int _port = TSVgetiById ( r, "id", "value", "port" );
    int _cd_s = TSVgetiById ( r, "id", "value", "cd_s" );
    int _cd_ns = TSVgetiById ( r, "id", "value", "cd_ns" );
    int _conn_num = TSVgetiById ( r, "id", "value", "conn_num" );
    int _max_retry = TSVgetiById ( r, "id", "value", "max_retry" );
    int _serial_rate = TSVgetiById ( r, "id", "value", "serial_rate" );
    char *_serial_config = TSVgetvalueById ( r, "id", "value", "serial_config" );
    char *_serial_pattern = TSVgetvalueById ( r, "id", "value", "serial_pattern" );
    char *_db_prog_path = TSVgetvalueById ( r, "id", "value", "db_prog_path" );
    if ( TSVnullreturned ( r ) ) {
        return 0;
    }
    *port = _port;
    cd->tv_sec = _cd_s;
    cd->tv_nsec = _cd_ns;
    *conn_num = _conn_num;
    *max_retry = _max_retry;
    *serial_rate = _serial_rate;
    *serial_config = _serial_config;
    *serial_pattern = _serial_pattern;
    *db_prog_path = _db_prog_path;
    return 1;
}


void *serverThreadFunction ( void *arg ) {
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
            int fd_conn = connection->fd_conn;
            SERVER_READ_CMD
            //printf("command: %s\n", cmd);
            if ( CMD_IS ( ACPP_CMD_CHANNEL_GET_FTS ) ) {
				SERVER_READ_I1LIST(channel_list.max_length)
				SERVER_RESPONSE_DEF_FTSLIST(channel_list.max_length)
				
		        FORLISTN ( i1l, i ) {
					Channel *item = NULL;
					LIST_GETBYID(item, &channel_list, i1l.item[i])
					if(item!=NULL){
						if(item->thread == NULL){
						//	printdo("skipping channel with no thread: %d\n", item->id);
							continue;
						}
						lockMutex(&item->run.mutex);
						double v=item->run.input;
						struct timespec tm = item->run.input_tm;
						int success = item->run.input_state;
						unlockMutex(&item->run.mutex);
						SERVER_RESPONSE_FTSLIST_PUSH(item->id, v, tm, success)
					}
				}
				//printdo("fts list l: %d\n", ftslr.length);
		        SERVER_RESPONSE_FTSLIST_SEND
		        SERVER_GOTO_STOP
		    } else if ( CMD_IS ( ACPP_CMD_APP_PRINT ) ) {
		        printData ( fd_conn );
		        SERVER_GOTO_STOP
		    } else if ( CMD_IS ( ACPP_CMD_CHANNEL_RESET ) ) {
				SERVER_READ_I1LIST(channel_list.max_length)
		        FORLISTN ( i1l, i ) {
					Channel *item = NULL;
					LIST_GETBYID(item, &channel_list, i1l.item[i])
					if(item!=NULL){
						if ( lockMutex ( &item->mutex ) ) {
		                    item->state = INIT;
		                    unlockMutex ( &item->mutex );
		                }
					}
				}
				SERVER_GOTO_STOP
		    } else {
		        putsde ( "unknow command\n" );
		    }
            SERVERM_STOP
            break;}
        default:
            pthread_exit ( NULL );
            break;
        }
    }

}


int initApp() {
    if ( !readSettings ( &config_tsv, CONFIG_FILE, &sock_port, &serial_thread_starter.thread_cd, &conn_num, &max_retry,  &serial_thread_starter.serial_rate,  &serial_thread_starter.serial_config, &serial_thread_starter.serial_pattern, &db_prog_path) ) {
        putsde ( "failed to read settings\n" );
        return 0;
    }
    if ( !initMutex ( &db_mutex ) ) {
        putsde ( "failed to initialize db mutex\n" );
        return 0;
    }
    if ( !initMutex ( &serial_thread_list_mutex ) ) {
        putsde ( "failed to initialize serial_thread_list mutex\n" );
        return 0;
    }
    if ( !serverm_init(&server_fd, sock_port, conn_num, &sc_list, serverThreadFunction)){
		putsde ( "failed to initialize multythreaded server\n" );
        return 0;
	 }
    return 1;
}

void appRun (int *state, int init_state) {
	serverm_accept ( server_fd, &sc_list );
}

int initData() {
	sqlite3 *db;
    if ( !db_openR ( db_prog_path , &db ) ) {
        putsde ( "failed to open DB\n" );
        return 0;
    }
    if ( !initChannelList ( &channel_list, max_retry, db ) ) {
        freeChannelList ( &channel_list );
        db_close ( db );
        goto failed;
    }
    db_close ( db );
    if ( !initSerialThreadStarter ( &serial_thread_starter ) ) {
		freeChannelList ( &channel_list );
        goto failed;
    }
    return 1;
failed:
    return 0;
}

void freeData() {
	serverm_free(server_fd, &sc_list);
	STOP_ALL_LLIST_THREADS(&serial_thread_list, SerialThread)
	freeThreadList ( &serial_thread_list );
	freeSerialThreadStarter(&serial_thread_starter);
}

void freeApp() {
    close ( server_fd );
    freeData();
    freeMutex ( &serial_thread_list_mutex );
    TSVclear ( &config_tsv );
}

void exit_nicely ( ) {
    freeApp();
    putsdo ( "\nexiting now...\n" );
    exit ( EXIT_SUCCESS );
}

int main ( int argc, char** argv ) {
#ifndef MODE_DEBUG
    daemon ( 0, 0 );
#endif
    conSig ( &exit_nicely );
    int data_initialized = 0;
    while ( 1 ) {
#ifdef MODE_DEBUG
     //   printf ( "%s(): %s %d\n", F, getAppState ( app_state ), data_initialized );
#endif
        switch ( app_state ) {
        case APP_RUN:
            appRun ( &app_state, data_initialized );
            break;
        case APP_INIT:
            if ( !initApp() ) {
                return ( EXIT_FAILURE );
            }
            app_state = APP_INIT_DATA;
            break;
        case APP_INIT_DATA:
            data_initialized = initData();
            app_state = APP_RUN;
            delayUsIdle ( 1000000 );
            break;
        case APP_STOP:
            freeData();
            data_initialized = 0;
            app_state = APP_RUN;
            break;
        case APP_RESET:
            freeApp();
            delayUsIdle ( 1000000 );
            data_initialized = 0;
            app_state = APP_INIT;
            break;
        case APP_EXIT:
            exit_nicely();
            break;
        default:
            freeApp();
            putsde ( "unknown application state\n" );
            return ( EXIT_FAILURE );
        }
    }
    freeApp();
    putsde ( "unexpected while break\n" );
    return ( EXIT_FAILURE );
}
