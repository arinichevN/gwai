#include "main.h"

static int app_state = APP_INIT;

static Mutex db_mutex = MUTEX_INITIALIZER;

static int sock_port = -1;
static int conn_num = 0;
static int max_retry;
static Serverm server;

static Mutex serial_thread_list_mutex = MUTEX_INITIALIZER;
static SerialThreadStarter serial_thread_starter;
static SerialThreadLList serial_thread_list = LLIST_INITIALIZER;
static ChannelList channel_list = LIST_INITIALIZER;

#include "util.c"
#include "channel.c"
#include "serialThread.c"
#include "serialThreadStarter.c"
#include "data.c"

int readSettings ( const char *data_path, int *port, struct timespec *cd, int *conn_num, int *max_retry, int *serial_rate, char *serial_config, char *serial_pattern) {
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, data_path ) ) {
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
    if ( TSVnullreturned ( r ) ) {
		TSVclear ( r );
        return 0;
    }
    *port = _port;
    cd->tv_sec = _cd_s;
    cd->tv_nsec = _cd_ns;
    *conn_num = _conn_num;
    *max_retry = _max_retry;
    *serial_rate = _serial_rate;
    strncpy(serial_config, _serial_config, LINE_SIZE);
    strncpy(serial_pattern, _serial_pattern, LINE_SIZE);
    TSVclear ( r );
    return 1;
}

void serveRequest(int SERVER_FD, const char *SERVER_CMD){
	printdo("command %s\n", SERVER_CMD);
    if(channelListHasGetCmd(&channel_list, SERVER_CMD)){
        SERVER_READ_I1LIST(channel_list.max_length)
        FORLISTN ( i1l, i ) {
			Channel *channel = NULL;
			LIST_GETBYID(channel, &channel_list, i1l.item[i])
			if(channel!=NULL){
				FORLISTN(channel->data_list, j){
                    SlaveDataItem *item = &channel->data_list.item[j];
                    if(strncmp(SERVER_CMD, item->cmd, SLAVE_CMD_MAX_SIZE) == 0){
                        if(item->result == 1 && item->sendFunction!=NULL){
                            item->sendFunction(SERVER_FD, channel->id, &item->mutex, &item->data);
                        }
                    }
                }
			}
		}
        SERVER_SEND_END
        return;
    }
    if(channelListHasSetCmd(&channel_list, SERVER_CMD)){
		SERVER_READ_I1S1LIST(channel_list.max_length)
		FORLISTN ( i1s1l, i ) {
			Channel *channel = NULL;
			LIST_GETBYID(channel, &channel_list, i1s1l.item[i].p0)
			if(channel!=NULL && channel->thread != NULL){
				FORLISTN(channel->set_list, j){
                    SlaveSetItem *item = &channel->set_list.item[j];
                    if(strncmp(SERVER_CMD, item->cmd, SLAVE_CMD_MAX_SIZE) == 0 && item->setFunction!=NULL){
                        switch(item->data_type){
							case SLAVE_TYPE_INT:
								{int v = atoi(i1s1l.item[i].p1);printdo("FOUTND int command %s %d\n", item->cmd, v);
								item->setFunction(channel->id, channel->thread->fd, &channel->thread->mutex, item->cmd, (void *) &v);
								break;}
							case SLAVE_TYPE_DOUBLE:
								{double v = atof(i1s1l.item[i].p1);printdo("FOUTND float command %s %f\n", item->cmd, v);
								if(!checkFloat(v)) break;
	                            item->setFunction(channel->id, channel->thread->fd, &channel->thread->mutex, item->cmd, (void *) &v);
	                            break;}
	                        default:
		                        break;
                        }
                    }
                }
			}
		}
		return;
	}
    if ( CMD_IS ( ACPP_CMD_APP_PRINT ) ) {
        printData ( SERVER_FD );
    } else if ( CMD_IS ( ACPP_CMD_APP_RESET ) ) {
		app_state = APP_RESET;
    } else if ( CMD_IS ( ACPP_CMD_CHANNEL_RESET ) ) {
		SERVER_READ_I1LIST(channel_list.max_length)
        FORLISTN ( i1l, i ) {
			Channel *channel = NULL;
			LIST_GETBYID(channel, &channel_list, i1l.item[i])
			if(channel!=NULL){
				if ( lockMutex ( &channel->mutex ) ) {
					channel->state = INIT;
                    unlockMutex ( &channel->mutex );
                }
			}
		}
    } else {//get-command for raw output
		printdo("try to parse RAW get command: %s\n", SERVER_CMD);
        SERVER_READ_I1LIST(channel_list.max_length)
        FORLISTN ( i1l, i ) {
			Channel *channel = NULL;
			LIST_GETBYID(channel, &channel_list, i1l.item[i])
			if(channel!=NULL && channel->thread!=NULL){
				printdo("RAW get command: %s\n", SERVER_CMD);
				char buf[SLAVE_DATA_BUFFER_LENGTH];
                channelSlaveGetRaw(channel, SERVER_CMD, buf, SLAVE_DATA_BUFFER_LENGTH);
                channelSendRawData (SERVER_FD,  channel->id, buf, SLAVE_DATA_BUFFER_LENGTH );
                
			}
		}
		SERVER_SEND_END
		return;
    }
}


int initApp() {
    if ( !readSettings ( CONFIG_FILE, &sock_port, &serial_thread_starter.thread_cd, &conn_num, &max_retry,  &serial_thread_starter.serial_rate,  serial_thread_starter.serial_config, serial_thread_starter.serial_pattern) ) {
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
    if ( !serverm_init(&server, sock_port, conn_num, serveRequest)){
		putsde ( "failed to initialize multythreaded server\n" );
        return 0;
	 }
    return 1;
}


int initData() {
    if ( !initChannelList ( &channel_list, max_retry, CHANNELS_CONFIG_FILE) ) {
        freeChannelList ( &channel_list );
        goto failed;
    }
    if ( !initSerialThreadStarter ( &serial_thread_starter ) ) {
		freeChannelList ( &channel_list );
        goto failed;
    }
    return 1;
failed:
    return 0;
}

void freeData() {
	serverm_free(&server);
	STOP_ALL_LLIST_THREADS(&serial_thread_list, SerialThread)
	freeChannelList ( &channel_list );
	freeThreadList ( &serial_thread_list );
	freeSerialThreadStarter(&serial_thread_starter);
}

void freeApp() {
    freeData();
    freeMutex ( &serial_thread_list_mutex );
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
    while ( 1 ) {
       // printdo ( "%s(): %s\n", F, getAppState ( app_state ) );
        switch ( app_state ) {
        case APP_RUN:
            nanosleep ( &(struct timespec) {0,10000000}, NULL );
            break;
        case APP_INIT:
            if ( !initApp() ) {
                return ( EXIT_FAILURE );
            }
            app_state = APP_INIT_DATA;
            break;
        case APP_INIT_DATA:
            initData();
            app_state = APP_RUN;
            delayUsIdle ( 1000000 );
            break;
        case APP_STOP:
            freeData();
            app_state = APP_RUN;
            break;
        case APP_RESET:
            freeApp();
            delayUsIdle ( 1000000 );
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
