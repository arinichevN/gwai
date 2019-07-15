#include "main.h"

#define CMD_GET_FTS "cgfts"


#define FV "%lf"
#define PDB PUART_DELIMITER_BLOCK_STR
#define PDE PUART_DELIMITER_END_STR


int channelGetFTSData ( RunData *item, int channel_id, int fd ) {
	int r = puart_sendCmd (fd, channel_id, CMD_GET_FTS);
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		return PUART_CONNECTION_FAILED;
	}
	size_t resp_len = 128;
	char response[resp_len];
    memset(response, 0, resp_len);
	r = puart_readResponse(fd, response, resp_len);
	puart_sendEnd(fd);
    if(r < 0){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		return r;
	}
	int id=-1;
	lockMutex(&item->mutex);
	int tm;
	r = sscanf(response, "%d" PDB FV PDB "%d" PDB "%d" PDE, &id, &item->input, &item->input_state, &tm);
	int nr = 4;
	if(r != nr){
		printde("read fts: failed to parse response (found:%d, need:%d)\n", r, nr);
		unlockMutex(&item->mutex);
		return 0;
	}
	if (tm != item->input_stm){
		item->input_stm = tm;
		item->input_tm = getCurrentTime();
	}
	unlockMutex(&item->mutex);
	if(id != channel_id){
		printde("read fts: expected %d id but returned %d\n", channel_id, id);
		return 0;
	}
	return 1;
}

int channelControlRun( RunData *item, int channel_id, int fd ) {
	int r = 1;
    switch ( item->state ) {
    case WAIT:
        if ( ton ( &item->tmr ) ) {
            item->state = SLAVE_GET;
        }
        break;
	case SLAVE_GET:
		r = channelGetFTSData ( item, channel_id, fd );
		item->state = WAIT;
        break;
	case OFF:
        break;
    case FAILURE:
        break;
    case INIT:
        tonSetInterval ( item->interval, &item->tmr );
        tonReset ( &item->tmr );
        item->tm = getCurrentTime();
        item->input_state = 0;
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
		r = channelControlRun(&item->run, item->id, fd);
		if(r == PUART_NO_RESPONSE){
			if(item->retry < item->max_retry){
				item->retry++;
				r = PUART_NO_RESPONSE;
			}
		}else{
			item->retry = 0;
		}
        break;
	case OFF:
        break;
    case FAILURE:
        break;
    case INIT:
	    item->retry = 0;
	    item->run.state = INIT;
        item->state = RUN;
        break;
    default:
        break;
    }
    unlockMutex(&item->mutex);
    return r;
    
}

#define ADC ACPP_DELIMITER_COLUMN_STR

void printChannelHeader(){
	putsdo("+----+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");
	putsdo("| id |   state   | run_state |   in_val  |  in_state |  in_stm   |   in_tms  |  in_tmns  |\n");
	putsdo("+----+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");
}
void printChannel(Channel *item){
	printdo("|%4d|%11s|%11s|%11.3f|%11d|%11d|%11ld|%11ld|\n", item->id, getStateStr(item->state), getStateStr(item->run.state), item->run.input, item->run.input_state, item->run.input_stm, item->run.input_tm.tv_sec, item->run.input_tm.tv_nsec)  ;
}
void printChannelFooter(){
	putsdo("+----+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n");
	putsdo("\n");
}

