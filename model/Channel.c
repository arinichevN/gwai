#include "Channel.h"

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
        FREE_LIST(&LIi.gcmd_list);
        FREE_LIST(&LIi.tgcmd_list);
        FREE_LIST(&LIi.igcmd_list);
        FREE_LIST(&LIi.scmd_list);
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
	item->state = INIT;
	unlockMutex(&item->mutex);
}

SlaveGetCommand *channel_getIntervalGetCmd(Channel *channel, const char *cmd){
	FORLISTN(channel->igcmd_list, j){
		SlaveGetCommand *item = &channel->igcmd_list.item[j].command;
		if(strncmp(cmd, item->name, SLAVE_CMD_MAX_SIZE) == 0){
		   return item;
		}
	}
    return NULL;
}

SlaveGetCommand *channel_getGetCmd(Channel *channel, const char *cmd){
	FORLISTN(channel->gcmd_list, j){
		SlaveGetCommand *item = &channel->gcmd_list.item[j];
		if(strncmp(cmd, item->name, SLAVE_CMD_MAX_SIZE) == 0){
		   return item;
		}
	}
    return NULL;
}

SlaveGetCommand *channel_getTextGetCmd(Channel *channel, const char *cmd){
	FORLISTN(channel->tgcmd_list, j){
		SlaveGetCommand *item = &channel->tgcmd_list.item[j];
		if(strncmp(cmd, item->name, SLAVE_CMD_MAX_SIZE) == 0){
		   return item;
		}
	}
    return NULL;
}

SlaveSetCommand *channel_getSetCmd(Channel *channel, const char *cmd){
	FORLISTN(channel->scmd_list, j){
		SlaveSetCommand *item = &channel->scmd_list.item[j];
		if(strncmp(cmd, item->name, SLAVE_CMD_MAX_SIZE) == 0){
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
		LIST_RESET(&LIi.gcmd_list)
        LIST_RESET(&LIi.tgcmd_list)
        LIST_RESET(&LIi.igcmd_list)
        LIST_RESET(&LIi.scmd_list)
	}
    for ( int i = 0; i < LML; i++ ) {
        LIi.id = TSVgetis ( r, i, "id" );
        char *get_file = TSVgetvalues ( r, i, "get" );
        char *tget_file = TSVgetvalues ( r, i, "tget" );
        char *iget_file = TSVgetvalues ( r, i, "iget" );
        char *set_file = TSVgetvalues ( r, i, "set" );
        if ( TSVnullreturned ( r ) ) {
            break;
        }
        if(!sgcList_init(&LIi.gcmd_list, get_dir, get_file, file_type)){
			TSVclear ( r );
            goto failed;
        }
        if(!sgcList_init(&LIi.tgcmd_list, tget_dir, tget_file, file_type)){
			TSVclear ( r );
            goto failed;
        }
        if(!sigcList_init(&LIi.igcmd_list, iget_dir, iget_file, file_type)){
			TSVclear ( r );
            goto failed;
        }
        if(!sscList_init(&LIi.scmd_list, set_dir, set_file, file_type)){
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
		LIi.state = INIT;
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

int channel_control(Channel *item, int fd){
	int r = 1;
	lockMutex(&item->mutex);
	switch ( item->state ) {
		case RUN:
			if(item->thread == NULL){
				item->state = OFF;
				printde ( "channel: self disabled, id=%d\n", item->id );
				break;
			}
	        FORLISTN(item->igcmd_list, i){
	            r = sigc_control(&item->igcmd_list.item[i], fd, &item->thread->mutex, item->id );
	            if(r == ACP_ERROR_CONNECTION){
	                if(item->retry < item->max_retry){
	                    item->retry++;
	                }else{
						break;
					}
	            }else{
	                item->retry = 0;
	            }
	        }
	        break;
		case OFF:
	        break;
	    case FAILURE:
	        break;
	    case INIT:
		    item->retry = 0;
	        item->state = RUN;
	        break;
	    default:
	        break;
    }
    unlockMutex(&item->mutex);
    return r;
    
}

#define ADC ACP_DELIMITER_COLUMN_STR


