#include "main.h"

#define CMD_GET_FTS "cgfts"

#define FV "%lf"
#define PDB PUART_DELIMITER_BLOCK_STR
#define PDE PUART_DELIMITER_END_STR

int channelListHasGetCmd(ChannelList *list, const char *cmd){
    FORLi{
        FORLISTN(LIi.data_list, j){
            SlaveDataItem *item = &LIi.data_list.item[j];
            if(strncmp(cmd, item->cmd, SLAVE_CMD_MAX_SIZE) == 0){
               return 1;
            }
        }
    }
    return 0;
}

int channelListHasSetCmd(ChannelList *list, const char *cmd){
    FORLi{
        FORLISTN(LIi.set_list, j){
            SlaveSetItem *item = &LIi.set_list.item[j];
            if(strncmp(cmd, item->cmd, SLAVE_CMD_MAX_SIZE) == 0){
               return 1;
            }
        }
    }
    return 0;
}

int channelSlaveGetFTSData (int fd, int channel_id, char *cmd,  Mutex *mutex, SlaveDType *data ) {
	int r = puart_sendCmd (fd, channel_id, cmd);
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		return PUART_CONNECTION_FAILED;
	}
	char response[SLAVE_DATA_BUFFER_LENGTH];
    memset(response, 0, SLAVE_DATA_BUFFER_LENGTH);
	r = puart_readResponse(fd, response, SLAVE_DATA_BUFFER_LENGTH);
	puart_sendEnd(fd);
    if(r < 0){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		return r;
	}
	int id=-1;
    double value;
    int state;
	unsigned int tm;
	r = sscanf(response, "%d" PDB FV PDB "%u" PDB "%d" PDE, &id, &value, &tm, &state );
	int nr = 4;
	if(r != nr){
		printde("read fts: failed to parse response (found:%d, need:%d)\n", r, nr);
		return 0;
	}
    if(id != channel_id){
		printde("read fts: expected %d id but returned %d\n", channel_id, id);
		return 0;
	}
    lockMutex(mutex);
	msec2timespec(&data->fts.tm, tm);
    data->fts.value = value;
    data->fts.state = state;
	unlockMutex(mutex);
	return 1;
}

int channelSlaveGetRaw (Channel *channel, const char *cmd,  char *data, int len ) {
	Mutex *mutex = &channel->thread->mutex;
	int fd = channel->thread->fd;
	lockMutex(mutex);
	int r = puart_sendCmd (fd, channel->id, cmd);
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		unlockMutex(mutex);
		return PUART_CONNECTION_FAILED;
	}
    memset(data, 0, len * sizeof (*data));
	r = puart_readResponse(fd, data, len);
	puart_sendEnd(fd);
	unlockMutex(mutex);
    if(r < 0){
		printde("\tcommunication error where channel_id=%d\n", channel->id);
		return r;
	}
	return 1;
}

int channelSlaveGetIntData (int fd,  int channel_id, char *cmd, Mutex *mutex, SlaveDType *data ) {
    return 1;
}

int channelSlaveGetFloatData (int fd,  int channel_id, char *cmd, Mutex *mutex, SlaveDType *data ) {
    return 1;
}

int channelSlaveSetFloatData (int channel_id, int fd, Mutex *mutex, char *cmd,  void *data ) {
	lockMutex(mutex);
	double *vp = data;
	int r = puart_sendDouble (fd, channel_id, cmd, *vp);
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		unlockMutex(mutex);
		return PUART_CONNECTION_FAILED;
	}
	unlockMutex(mutex);
	return 1;
}

int channelSlaveSetIntData (int channel_id, int fd, Mutex *mutex, char *cmd, void *data ) {
	lockMutex(mutex);
	int *vp = data;
	int r = puart_sendInt (fd, channel_id, cmd, *vp);
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		unlockMutex(mutex);
		return PUART_CONNECTION_FAILED;
	}
	unlockMutex(mutex);
	return 1;
}

void channelSendFloatData (int fd,  int channel_id, Mutex *mutex, SlaveDType *data ) {
    char q[ACPP_ROW_MAX_LENGTH];
    lockMutex(mutex);
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT ACPP_DELIMITER_ROW_STR, channel_id, data->dbl );
    unlockMutex(mutex);
    acpp_send ( q, fd );
}

void channelSendIntData (int fd,  int channel_id, Mutex *mutex, SlaveDType *data ) {
    char q[ACPP_ROW_MAX_LENGTH];
    lockMutex(mutex);
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, channel_id, data->intg );
    unlockMutex(mutex);
    acpp_send ( q, fd );
}

void channelSendFTSData (int fd,  int channel_id, Mutex *mutex, SlaveDType *data ) {
    char q[ACPP_ROW_MAX_LENGTH];
    lockMutex(mutex);
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, channel_id, data->fts.value, data->fts.tm.tv_sec, data->fts.tm.tv_nsec, data->fts.state );
    unlockMutex(mutex);
    acpp_send ( q, fd );
}

void channelSendRawData (int fd,  int channel_id, char *data, int len ) {
    char q[ACPP_ROW_MAX_LENGTH];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%s" ACPP_DELIMITER_ROW_STR, channel_id, data);
    acpp_send ( q, fd );
}

int channelDataItemControl( int fd, int channel_id, SlaveDataItem *item ) {
	int r = 1;
    switch ( item->state ) {
    case WAIT:
        if ( ton ( &item->tmr ) ) {
            item->result = item->readFunction(fd, channel_id, item->cmd, &item->mutex, &item->data);
        }
        break;
	case OFF:
        break;
    case FAILURE:
        break;
    case INIT:
        tonSetInterval ( item->interval, &item->tmr );
        tonReset ( &item->tmr );
        item->state = WAIT;
        break;
    default:
        break;
    }
    return r;
}


int channelControl(Channel *item, int fd){
	int r = 1;
	lockMutex(&item->mutex);
	switch ( item->state ) {
	case RUN:
        FORLISTN(item->data_list, i){
            r = channelDataItemControl(fd, item->id, &item->data_list.item[i]);
            if(r == PUART_NO_RESPONSE){
                if(item->retry < item->max_retry){
                    item->retry++;
                    r = PUART_NO_RESPONSE;
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

#define ADC ACPP_DELIMITER_COLUMN_STR


