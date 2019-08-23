#include "main.h"

char * getStateStr ( int state ) {
    switch ( state ) {
    case OFF:
        return "OFF";
    case INIT:
        return "INIT";
    case RUN:
        return "RUN";
    case DO:
        return "DO";
	case TERMINATED:
        return "TERMINATED";
   case SEARCH_NEED:
	   return "SEARCH_NEED";
   case SEARCH_PATH:
	   return "SEARCH_PATH";
   case FIND_CHANNELS:
	   return "FIND_CHANNELS";
	case SLAVE_GET:
        return "SLAVE_GET";
	case SLAVE_RESET:
        return "SLAVE_RESET";
	case SLAVE_COOP:
        return "SLAVE_COOP";
    case BUSY:
        return "BUSY";
    case IDLE:
        return "IDLE";
    case WAIT:
        return "WAIT";
    case OPENED:
        return "OPENED";
    case CLOSED:
        return "CLOSED";
    case OPEN:
        return "OPEN";
    case CLOSE:
        return "CLOSE";
    case DISABLE:
        return "DISABLE";
    case UNDEFINED:
        return "UNDEFINED";
    case FAILURE:
        return "FAILURE";
    }
    return "\0";
}

char * getDataTypeStr ( int v ) {
    switch ( v ) {
		case SLAVE_TYPE_FTS: 
			return "FTS";
	    case SLAVE_TYPE_INT:
		    return "INT";
	    case SLAVE_TYPE_DOUBLE:
		    return "DOUBLE";
	    case SLAVE_TYPE_UNKNOWN:
		    return "SUNKNOWN";
	}
    return "\0";
}


void printData ( int fd ) {
    char q[LINE_SIZE*3];
    snprintf ( q, sizeof q, "CONFIG_FILE: %s\n", CONFIG_FILE );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "CHANNELS_CONFIG_FILE: %s\n", CHANNELS_CONFIG_FILE );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "port: %d\n", sock_port );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "conn_num: %d\n", conn_num );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "max_retry: %d\n", max_retry );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "app_state: %s\n", getAppState ( app_state ) );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "PID: %d\n", getpid() );
    SEND_STR ( q )
    
    SEND_STR ( "+----------------------------------------------------------------------------+\n" )
    SEND_STR ( "|                           serial thread starter                            |\n" )
    SEND_STR ( "+-----------+----------------+-----------+-----------+-----------+-----------+\n" )
    SEND_STR ( "|   state   | serial_pattern |serial_rate|serial_conf|    cd_s   |   cd_ns   |\n" )
    SEND_STR ( "+-----------+----------------+-----------+-----------+-----------+-----------+\n" )
	snprintf ( q, sizeof q, "|%11s|%16s|%11d|%11s|%11ld|%11ld|\n",
	   getStateStr(serial_thread_starter.state),
	   serial_thread_starter.serial_pattern,
	   serial_thread_starter.serial_rate,
	   serial_thread_starter.serial_config,
	   serial_thread_starter.thread_cd.tv_sec,
	   serial_thread_starter.thread_cd.tv_nsec
	 );
	SEND_STR ( q )
    SEND_STR ( "+-----------+----------------+-----------+-----------+-----------+-----------+\n" )
    
	SEND_STR ( "+------------------------------------------------------------------------------------------------------+\n" )
    SEND_STR ( "|                                          serial thread                                               |\n" )
    SEND_STR ( "+-----------+-------------+-----------+-----------+----------------+-----------+-----------+-----------+\n" )
    SEND_STR ( "|     ptr   |    state    |   cd_sec  |  cd_nsec  |  serial_path   |serial_rate|   retry   | max_retry |\n" )
    SEND_STR ( "+-----------+-------------+-----------+-----------+----------------+-----------+-----------+-----------+\n" )
    FOREACH_LLIST(item, &serial_thread_list, SerialThread) {
        snprintf ( q, sizeof q, "|%11p|%13s|%11ld|%11ld|%16s|%11d|%11d|%11d|\n",
                   (void *)item,
                   getStateStr(item->state),
                   item->cycle_duration.tv_sec,
                   item->cycle_duration.tv_nsec,
                   item->serial_path,
                   item->serial_rate,
                   item->retry,
                   item->max_retry
                 );
        SEND_STR ( q )
    }
    SEND_STR ( "+-----------+-------------+-----------+-----------+----------------+-----------+-----------+-----------+\n" )
    
    SEND_STR ( "+-----------------------------------------------------------+\n" )
    SEND_STR ( "|                         channel                           |\n" )
    SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+\n" )
    SEND_STR ( "|     id    | thread_pt |   state   |   retry   | max_retry |\n" )
    SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+\n" )
    FORLISTN ( channel_list, i ) {
		Channel *item = &channel_list.item[i];
        snprintf ( q, sizeof q, "|%11d|%11p|%11s|%11d|%11d|\n",
                   item->id,
                   (void *)item->thread,
                   getStateStr(item->state),
                   item->retry,
                   item->max_retry
                 );
        SEND_STR ( q )
    }
    SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+\n" )
    	
	
   
	SEND_STR ( "+-----------------------------------------------------------------------------------+\n" )
    SEND_STR ( "|                                  channel                                          |\n" )
    SEND_STR ( "|           +-----------------------------------------------------------------------+\n" )
    SEND_STR ( "|           |                         slave poll command                            |\n" )
    SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n" )
    SEND_STR ( "|     id    |    cmd    |   type    |interval_s |interval_ns|  result   |   state   |\n" )
    SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n" )
    FORLISTN ( channel_list, i ) {
		Channel *channel = &channel_list.item[i];
		FORLISTN(channel->data_list, j){
			SlaveDataItem *item = &channel->data_list.item[j];
	        snprintf ( q, sizeof q, "|%11d|%11s|%11s|%11ld|%11ld|%11d|%11s|\n",
	                   channel->id,
	                   item->cmd,
	                   getDataTypeStr(item->data_type),
	                   item->interval.tv_sec,
	                   item->interval.tv_nsec,
	                   item->result,
	                   getStateStr(item->state)
	                 );
	        SEND_STR ( q )
		}
	}
	SEND_STR ( "+-----------+-----------+-----------+-----------+-----------+-----------+-----------+\n" )
	
	SEND_STR ( "+-----------------------------------+\n" )
    SEND_STR ( "|              channel              |\n" )
    SEND_STR ( "|           +-----------------------+\n" )
    SEND_STR ( "|           |   slave set command   |\n" )
    SEND_STR ( "+-----------+-----------+-----------+\n" )
    SEND_STR ( "|     id    |    cmd    |   type    |\n" )
    SEND_STR ( "+-----------+-----------+-----------+\n" )
    FORLISTN ( channel_list, i ) {
		Channel *channel = &channel_list.item[i];
		FORLISTN(channel->set_list, j){
			SlaveSetItem *item = &channel->set_list.item[j];
	        snprintf ( q, sizeof q, "|%11d|%11s|%11s|\n",
	                   channel->id,
	                   item->cmd,
	                   getDataTypeStr(item->data_type)	                   
	                 );
	        SEND_STR ( q )
		}
	}
	SEND_STR ( "+-----------+-----------+-----------+\n" )
	
	
	SEND_STR ( "+-----------------------+\n" )
    SEND_STR ( "|      thread channels  |\n" )
    SEND_STR ( "+-----------+-----------+\n" )
    SEND_STR ( "|sthread_ptr| channel_id|\n" )
    SEND_STR ( "+-----------+-----------+\n" )
    FOREACH_LLIST(item, &serial_thread_list, SerialThread) {
		FOREACH_LLIST (channelptr, &item->channelptr_llist, ChannelPtr ) {
			Channel *channel = channelptr->item;
	        snprintf ( q, sizeof q, "|%11p|%11d|\n",
	                   (void *)item,
	                   channel->id
	                 );
	        SEND_STR ( q )
		}
    }
    SEND_STR ( "+-----------+-----------+\n\f" )
    
}


