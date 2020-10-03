#include "Channel.h"

int channel_RUN(Channel *item, int fd);

int channel_OFF(Channel *item, int fd);

int channel_FAILURE(Channel *item, int fd);

int channel_INIT(Channel *item, int fd);

int channel_check ( Channel *item ) {
    int success = 1;
    if ( 0 ) {
        printde ( "invalid: channel where id=%d\n", item->id );
        success = 0;
    }
    return success;
}

void channelList_free (ChannelList *list){
	FORLi{
        FORLISTN(LIi.igcmd_list, j){
            sigc_free(&LIi.igcmd_list.item[j]);
        }
        FREE_LIST(&LIi.igcmd_list);
		freeMutex ( &LIi.mutex );
	}
	FREE_LIST(list);
}

void channel_resetData(Channel *item){
	lockMutex(&item->mutex);
	FORLISTN(item->igcmd_list, i){
		sigc_reset(&item->igcmd_list.item[i]);
	}
	item->thread = NULL;
	item->retry = 0;
	item->control = channel_INIT;
	unlockMutex(&item->mutex);
}

SlaveGetCommand *channel_getIntervalGetCmd(Channel *channel, int cmd){
	FORLISTN(channel->igcmd_list, j){
		SlaveGetCommand *item = &channel->igcmd_list.item[j].command;
		if(cmd == item->id){
		   return item;
		}
	}
    return NULL;
}


int channel_slaveToClient (Channel *channel, char *pack_str, int tcp_fd) {
	Mutex *mutex = &channel->thread->mutex;
	int fd = channel->thread->fd;
	int channel_id = channel->id;
	//sending request to slave
	lockMutex(mutex);
	int r = acpserial_sendTcpPack(fd, pack_str);
	if(r != ACP_SUCCESS){
		unlockMutex(mutex);
		return r;
	}
	//reading slave response
	size_t resp_len = ACP_BUF_MAX_LENGTH;
	char response[resp_len];
    memset(response, 0, resp_len);
    CH_SLEEP_BEFORE_READ_SLAVE
	r = acpserial_readResponse(fd, response, resp_len);
	unlockMutex(mutex);
	if(r != ACP_SUCCESS){
		printde("communication error where channel_id=%d\n", channel_id);
		return r;
	}
	r = acpserial_checkCRC(response);
	if(r != ACP_SUCCESS){
		return r;
	}
	acptcp_convertSerialPack(response);
	//sending slave response to client
	acptcp_send ( response, tcp_fd );
	return ACP_SUCCESS;
}

int channel_slaveToClientText (Channel *channel, char *pack_str, int tcp_fd) {
	Mutex *mutex = &channel->thread->mutex;
	int fd = channel->thread->fd;
	//sending request to slave
	lockMutex(mutex);
	int r = acpserial_sendTcpPack(fd, pack_str);
	if(r != ACP_SUCCESS){
		unlockMutex(mutex);
		return r;
	}
	//from serial to inet
    CH_SLEEP_BEFORE_READ_SLAVE
	if(!serial_canRead(fd, ACPSERIAL_TIMEOUT_MS)){
		putsde("failed to read slave response\n");
		unlockMutex(mutex);
		return ACP_ERROR_NO_RESPONSE;
		//return ACP_ERROR_CONNECTION;
	}
	r = ACP_SUCCESS;
	while(1){
		char c;
		ssize_t nr = read(fd, &c, 1);
		if(nr < 1){
			break;
		}
		ssize_t nw = write ( tcp_fd, &c, 1 );
		if(nw < 1){
			r = ACP_ERROR;
			break;
		}
	}
	unlockMutex(mutex);
	return r;
}

int channel_sendRawDataToSlave (Channel *channel, char *pack_str ) {
	Mutex *mutex = &channel->thread->mutex;
	int fd = channel->thread->fd;
	lockMutex(mutex);
	int r = acpserial_sendTcpPack(fd, pack_str);
	unlockMutex(mutex);
	return r;
}

int channelList_init ( ChannelList *list, int max_retry, const char *config_path, const char *get_dir, const char *iget_dir, const char *tget_dir, const char *set_dir, const char *file_type  ) {
	RESET_LIST ( list )
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, config_path ) ) {
        TSVclear ( r );
        return 1;
    }
    int n = TSVntuples ( r );
    if ( n <= 0 ) {
        TSVclear ( r );
        putsde ( "no data rows in file\n" );
        return 0;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
		TSVclear ( r );
        putsde ( "failed to allocate memory for channel list\n" );
        return 0;
    }
    for ( int i = 0; i < LML; i++ ) {
        LIST_RESET(&LIi.igcmd_list)
	}
    for ( int i = 0; i < LML; i++ ) {
        LIi.id = TSVgetis ( r, i, "id" );
        char *iget_file = TSVgetvalues ( r, i, "iget" );
        if ( TSVnullreturned ( r ) ) {
            break;
        }
        if(!sigcList_init(&LIi.igcmd_list, iget_dir, iget_file, file_type)){
			TSVclear ( r );
            goto failed;
        }
        LL++;
    }
    TSVclear ( r );
    if ( list->length != list->max_length ) {
        printde ( "check file %s: list.length=%zu but %zu expected\n", config_path, list->length, list->max_length );
        goto failed;
    }
    FORLi{
		LIi.thread = NULL;
		LIi.max_retry = max_retry;
		LIi.control = channel_INIT;
		if ( !initMutex ( &LIi.mutex ) ) {
	        putsde ( "failed to initialize channel mutex\n" );
	        goto failed;
	    }
		if ( !channel_check ( &LIi ) ) {
	        goto failed;
	    }
	}
    return 1;
    failed:
    channelList_free(list);
    return 0;
}

void channel_setThread(Channel *item, struct sthread_st *thread){
	item->thread = thread;
}

void channel_start(Channel *item){
	item->control = channel_INIT;
}

const char *channel_getStateStr(Channel *item){
	if(item->control == channel_RUN) return "RUN";
	else if(item->control == channel_OFF) return "OFF";
	else if(item->control == channel_FAILURE) return "FAILURE";
	else if(item->control == channel_INIT) return "INIT";
	return "?";
}

int channel_RUN(Channel *item, int fd){
	int r = 1;
	lockMutex(&item->mutex);
	if(item->thread == NULL){
		item->control = channel_OFF;
		printde ( "channel: self disabled, id=%d\n", item->id );
		goto done;
	}
	FORLISTN(item->igcmd_list, i){
		r = sigc_control(&item->igcmd_list.item[i], fd, &item->thread->mutex, item->id );
		if(r == ACP_ERROR_CONNECTION){
			if(item->retry < item->max_retry){
				item->retry++;
			}else{
				goto done;
			}
		}else{
			item->retry = 0;
		}
	}
	done:
	unlockMutex(&item->mutex);
    return r;
}

int channel_OFF(Channel *item, int fd){
    return 1;
}

int channel_FAILURE(Channel *item, int fd){
    return 1;
}

int channel_INIT(Channel *item, int fd){
	lockMutex(&item->mutex);
	item->retry = 0;
	item->control = channel_RUN;
	unlockMutex(&item->mutex);
    return 1;
}

#define ADC ACP_DELIMITER_COLUMN_STR


