#include "SlaveIntervalGetCommand.h"

void sigc_free(SlaveIntervalGetCommand *item){
	freeMutex ( &item->command.mutex );
}

int sigcList_init(SlaveIntervalGetCommandList *list, const char *dir, const char *file_name, const char *file_type){
	RESET_LIST ( list )
	char path[LINE_SIZE];
	path[0] = '\0';
	strncat(path, dir, LINE_SIZE - 1);
	strncat(path, file_name, LINE_SIZE - strlen(path) - 1);
	strncat(path, file_type, LINE_SIZE - strlen(path) - 1);
	printf("iget path: %s\n", path);
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, path ) ) {
        TSVclear ( db );
        return 1;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		putsdo("\tno slave poll commands\n");
        return 1;
    }
    ALLOC_LIST ( list,nt );
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "\tfailed to allocate memory for channel poll list\n" );
        return 0;
    }
    for(int i = 0; i<nt; i++) {
		if(LL >= LML) break;
		int is = TSVgetis ( db, i, "interval_s" );
		int ins = TSVgetis ( db, i, "interval_ns" );
		char *cmd = TSVgetvalues(db, i, "cmd");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("\tnull returned while reading channel_poll file 2\n");
			return 0;
		}
		if(is < 0 || ins < 0){
			FREE_LIST(list);
			TSVclear ( db );
			putsde("\tchannel poll file: bad interval\n");
			return 0;
		}
		LIll.interval.tv_sec = is;
		LIll.interval.tv_nsec = ins;
		strncpy(LIll.command.name, cmd, SLAVE_CMD_MAX_SIZE);
		if ( !initMutex ( &LIll.command.mutex ) ) {
			FREE_LIST ( list );
			TSVclear ( db );
			putsde ( "\tfailed to initialize slave data mutex\n" );
			return 0;
		}
		LIll.state = INIT;
		LL++;
		//printf("init poll LL++\n");
    }
    //printf("poll LL=%d\n",LL);
    TSVclear ( db );
    return 1;
}

static int channelGetRawData (int fd, Mutex *mutex, int channel_id, const char *cmd,  char *data, int len ) {
	lockMutex(mutex);
	int r = acpserial_sendChCmd (fd, channel_id, cmd);
	if(r == ACP_ERROR_CONNECTION){
		unlockMutex(mutex);
		return r;
	}
    memset(data, 0, len * sizeof (*data));
    CH_SLEEP_BEFORE_READ_SLAVE
    r = acpserial_readResponse(fd, data, len);
    unlockMutex(mutex);
    if(r != ACP_SUCCESS){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		return r;
	}
    r = acpserial_checkCRC(data);
    if(r != ACP_SUCCESS){
		return r;
	}
	acptcp_convertSerialPack(data);
	return ACP_SUCCESS;
}

void sigc_reset(SlaveIntervalGetCommand *item){
	item->command.result = 0;
}

int sigc_control(SlaveIntervalGetCommand *item, int fd, int channel_id ) {
    switch ( item->state ) {
    case WAIT:
        if ( tonr( &item->tmr ) ) {
            item->command.result = channelGetRawData (fd, &item->command.mutex, channel_id,  item->command.name,  item->command.data, ACP_BUF_MAX_LENGTH);                    
        }
        break;
	case OFF:
        break;
    case FAILURE:
        break;
    case INIT:
        ton_setInterval ( item->interval, &item->tmr );
        ton_reset ( &item->tmr );
        item->state = WAIT;
        break;
    default:
        break;
    }
    return item->command.result;
}
