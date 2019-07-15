#include "cmd/main.h"
#include "data.h"
int acpp_cpPeer ( Peer *dest, const Peer * src ) {
    strncpy ( dest->id, src->id, ACPP_PEER_ID_LENGTH );
    strncpy ( dest->addr_str, src->addr_str, ACPP_PEER_ADDR_STR_LENGTH );
    dest->port=src->port;
    dest->fd=src->fd;
    if ( !acpp_makeClientAddr ( &dest->addr, dest->addr_str, dest->port ) ) {
        printde ( "bad ip address for peer with id=%s\n", dest->id );
        return 0;
    }
    dest->addr_size = sizeof dest->addr;
    return 1;
}

int acpp_initPeer ( Peer* item ) {
    if ( !acpp_makeClientAddr ( &item->addr, item->addr_str, item->port ) ) {
        printde ( "bad ip address for peer with id=%s\n", item->id );
        return 0;
    }
    item->addr_size = sizeof item->addr;
    return 1;
}

int acpp_initPeerList ( PeerList* list ) {
    FORLi {
        if ( !acpp_initPeer ( list->item + i ) ) {
            return 0;
        }
    }
    return 1;
}

int acpp_initRChannel ( RChannel* item ) {
    return acpp_initPeer ( &item->peer ) ;
}
int acpp_cpRChannel ( RChannel *dest, const RChannel *src ) {
    dest->id=src->id;
    dest->channel_id=src->channel_id;
    return acpp_cpPeer ( &dest->peer, &src->peer );
}

int acpp_getRChannelFromList ( RChannel *dest , const RChannelList *list, int id ) {
    RChannel *needle;
    LIST_GETBYID ( needle, list, id )
    if ( needle == NULL ) {
        return 0;
    }
    return acpp_cpRChannel ( dest, needle );
}
//*******************************************************************************************************************************

static int bufnrow ( char **v ) {
    for ( int i=0; (*v)[i] != '\0'; i++ ) {
        if ( ( *v ) [i] == ACPP_DELIMITER_ROW ) {
            *v = &( *v ) [i+1];
            return 1;
        }
    }
    return 0;
}

int acpp_buftodata ( char **v ) {
    for ( int i=0; i<ACPP_BUFFER_SIZE-1; i++ ) {
        if ( ( *v ) [i] == ACPP_DELIMITER_BLOCK ) {
            *v = &( *v ) [i+1];
            return 1;
        }
    }
    putsde ( "data not found\n" );
    return 0;
}

int acpp_getI ( char *s, int *data ) {
    if ( sscanf ( s, "%d", data ) != 1 ) {
        return 0;
    }
    return 1;
}

int acpp_getF ( char *s, double *data ) {
    if ( sscanf ( s, ACPP_FLOAT_FORMAT_IN, data ) != 1 ) {
        return 0;
    }
    return 1;
}

void acpp_getI1List ( char *s, void *data ) {
    I1List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int p0;
        if ( sscanf ( buff, "%d", &p0 ) != 1 ) {
            break;
        }
        list->item[list->length] = p0;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getI2List ( char *s, void *data ) {
    I2List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int p0, p1;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR "%d", &p0, &p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->length++;
       if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getI3List ( char *s, void *data ) {
    I3List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int p0, p1, p2;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d", &p0, &p1, &p2 ) != 3 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->item[list->length].p2 = p2;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getF1List ( char *s, void *data ) {
    F1List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        double p0;
        if ( sscanf ( buff, ACPP_FLOAT_FORMAT_IN, &p0 ) != 1 ) {
            break;
        }
        list->item[list->length] = p0;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getI1F1List ( char *s, void *data ) {
    I1F1List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int p0;
        double p1;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_IN, &p0, &p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getI1U321List ( char *s, void *data ) {
    I1U321List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int p0;
        uint32_t p1;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR "%u", &p0, &p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getS1List ( char *s, void *data ) {
    S1List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        char p0[LINE_SIZE];
        memset ( p0, 0, sizeof p0 );
        size_t i = 0;
        size_t j = 0;
        int f = 0;
        for ( i = 0; i < LINE_SIZE; i++ ) {
            if ( buff[0] == '\0' ) {
                f = 1;
                return;
            }
            if ( buff[0] == ACPP_DELIMITER_ROW ) {
                break;
            }
            p0[j] = buff[0];
            j++;
            buff = &buff[1];
        }
        strcpy ( &list->item[list->length * LINE_SIZE], p0 );
        list->length++;
        if ( f ) {
            return;
        }
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getI1S1List ( char *s, void *data ) {
    I1S1List *list = data;
    char *buff = s;
    list->length = 0;
    char format[LINE_SIZE];
    int n = sprintf ( format, "%%d%c%%%lus", ACPP_DELIMITER_COLUMN, LINE_SIZE );
    if ( n <= 0 ) {
        return;
    }
    while ( list->length < list->max_length ) {
        int p0;
        char p1[LINE_SIZE];
        memset ( p1, 0, sizeof p1 );
        if ( sscanf ( buff, format, &p0, p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        strcpy ( list->item[list->length].p1, p1 );
        list->length++;
       if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getFTSList ( char *s, void *data ) {
    FTSList *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int id, state;
        double temp;
        struct timespec tm;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_IN ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%d", &id, &temp, &tm.tv_sec, &tm.tv_nsec, &state ) != 5 ) {
            break;
        }
        list->item[list->length].id = id;
        list->item[list->length].value = temp;
        list->item[list->length].tm.tv_sec = tm.tv_sec;
        list->item[list->length].tm.tv_nsec = tm.tv_nsec;
        list->item[list->length].state = state;
        list->length++;
       if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getITSList ( char *s, void *data ) {
    ITSList *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        int id, state;
        int temp;
        struct timespec tm;
        if ( sscanf ( buff, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%d", &id, &temp, &tm.tv_sec, &tm.tv_nsec, &state ) != 5 ) {
            break;
        }
        list->item[list->length].id = id;
        list->item[list->length].value = temp;
        list->item[list->length].tm.tv_sec = tm.tv_sec;
        list->item[list->length].tm.tv_nsec = tm.tv_nsec;
        list->item[list->length].state = state;
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

void acpp_getS2List ( char *s, void *data ) {
    S2List *list = data;
    char *buff = s;
    list->length = 0;
    while ( list->length < list->max_length ) {
        char p0[LINE_SIZE];
        char p1[LINE_SIZE];
        memset ( p0, 0, sizeof p0 );
        memset ( p1, 0, sizeof p1 );
        size_t i = 0;
        size_t j = 0;
        int f = 0;
        for ( i = 0; i < LINE_SIZE; i++ ) {
            if ( buff[0] == ACPP_DELIMITER_COLUMN ) {
                buff = &buff[1];
                break;
            }
            if ( buff[0] == '\0' || buff[0] == ACPP_DELIMITER_ROW ) {
                f = 1;
                break;
            }
            p0[j] = buff[0];
            j++;
            buff = &buff[1];
        }
        if ( f ) {
            break;
        }
        j = 0;
        for ( i = 0; i < LINE_SIZE; i++ ) {
            if ( buff[0] == ACPP_DELIMITER_COLUMN ) {
                buff = &buff[1];
                break;
            }
            if ( buff[0] == '\0' || buff[0] == ACPP_DELIMITER_ROW ) {
                break;
            }
            p1[j] = buff[0];
            j++;
            buff = &buff[1];
        }
        strcpy ( list->item[list->length].p0, p0 );
        strcpy ( list->item[list->length].p1, p1 );
        list->length++;
        if(!bufnrow ( &buff )){
			return;
		}
    }
}

//------------------------------------------------------------------------------------------------------------------

void acpp_readI1List ( void *data, int fd ) {
    I1List *list = data;
    list->length = 0;
    while ( list->length < list->max_length ) {
		char row[ACPP_ROW_MAX_LENGTH];
		if(!acpp_readRow ( row, ACPP_ROW_MAX_LENGTH, fd )){
			break;
		}
        int p0;
        if ( sscanf ( row, "%d", &p0 ) != 1 ) {
            break;
        }
        list->item[list->length] = p0;
        list->length++;
    }
}

void acpp_readI1F1List ( void *data, int fd ) {
    I1F1List *list = data;
    list->length = 0;
    while ( list->length < list->max_length ) {
		char row[ACPP_ROW_MAX_LENGTH];
		if(!acpp_readRow ( row, ACPP_ROW_MAX_LENGTH, fd )){
			break;
		}
        int p0;
        double p1;
        if ( sscanf ( row, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_IN, &p0, &p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->length++;
    }
}

void acpp_readI2List ( void *data, int fd ) {
    I1F1List *list = data;
    list->length = 0;
    while ( list->length < list->max_length ) {
		char row[ACPP_ROW_MAX_LENGTH];
		if(!acpp_readRow ( row, ACPP_ROW_MAX_LENGTH, fd )){
			break;
		}
        int p0, p1;
        if ( sscanf ( row, "%d" ACPP_DELIMITER_COLUMN_STR "%d", &p0, &p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        list->item[list->length].p1 = p1;
        list->length++;
    }
}

void acpp_readI1S1List ( void *data, int fd ) {
	I1S1List *list = data;
    list->length = 0;
    char format[LINE_SIZE];
    int n = sprintf ( format, "%%d%c%%%lus", ACPP_DELIMITER_COLUMN, LINE_SIZE );
    if ( n <= 0 ) {
        return;
    }
    while ( list->length < list->max_length ) {
		char row[ACPP_ROW_MAX_LENGTH];
		if(!acpp_readRow ( row, ACPP_ROW_MAX_LENGTH, fd )){
			break;
		}
        int p0;
        char p1[LINE_SIZE];
        memset ( p1, 0, sizeof p1 );
        if ( sscanf ( row, format, &p0, p1 ) != 2 ) {
            break;
        }
        list->item[list->length].p0 = p0;
        strncpy ( list->item[list->length].p1, p1, LINE_SIZE );
        list->length++;
    }
}

//------------------------------------------------------------------------------------------------------------------
int acpp_setI1List ( char *s, void *data ) {
    I1List *list = data;
    FORLi {
        char q[LINE_SIZE];
        snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_ROW_STR, LIi );
        if ( !acpp_strCat ( s, q ) ) {
            putsde ( "failed to add string to request" );
            return 0;
        }
    }
    return 1;
}

int acpp_setI1F1List ( char *s, void *data ) {
    I1F1List *list = data;
    FORLi {
        char q[LINE_SIZE];
        snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT  ACPP_DELIMITER_ROW_STR, LIi.p0, LIi.p1 );
        if ( !acpp_strCat ( s, q ) ) {
            putsde ( "failed to add string to request" );
            return 0;
        }
    }
    return 1;
}

//*******************************************************************************************************************************

void acpp_sendFTSList(FTSList *list, int fd){
	FORLi{
		char q[ACPP_ROW_MAX_LENGTH];
		snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, LIi.id, LIi.value, LIi.tm.tv_sec, LIi.tm.tv_nsec, LIi.state );
		acpp_send ( q, fd );
	}
	acpp_send (ACPP_DELIMITER_END_STR, fd );
}
//*********************************************************************************************************************************
int acpp_sendRequest ( char *cmd, void *data, int ( *data_set_fun ) ( char *, void * ), int fd ) {
    ACPP_DEF_BUFFER ( request )
    if ( !acpp_setCmd ( request, cmd ) ) {
        putsde ( "failed to add command to request\n" );
        return 0;
    }
    if ( ! ( *data_set_fun ) ( request, data ) ) {
        putsde ( "failed to add data to request\n" );
        return 0;
    }
    if(!acpp_setEnd(request)){
		putsde ( "failed to add end sign\n" );
		return 0;
	}
    return acpp_send ( request, fd );
}

int acpp_readData ( void *data, void ( *data_get_fun ) ( char *, void * ), int fd ) {
    ACPP_DEF_BUFFER ( response );
    if ( !acpp_read ( response, fd ) ) {
        putsde ( "failed to read response\n" );
        return 0;
    }
    ( *data_get_fun ) ( response, data );
    return 1;
}



//**************************************************************************************************************************+++++++

int acpp_peerSendCmd ( const char *cmd, Peer *peer ) {
    if ( !acpp_initClient ( peer, 3 ) ) {
        putsde ( "client not initialized" );
        return 0;
    }
    ACPP_DEF_BUFFER ( request )
    if ( !acpp_setCmd ( request, cmd ) ) {
        putsde ( "failed to add command to request\n" );
		close(peer->fd);
        return 0;
    }
    if(!acpp_setEnd(request)){
		putsde ( "failed to add end sign\n" );
		close(peer->fd);
		return 0;
	}
    return acpp_send ( request, peer->fd );
}


int acpp_peerGetFTSList ( FTSList *output, Peer *peer, I1List *input ) {
    if ( !acpp_initClient ( peer, 3 ) ) {
        putsde ( "client not initialized" );
        return 0;
    }
    if ( !acpp_sendRequest ( ACPP_CMD_CHANNEL_GET_FTS, input, &acpp_setI1List, peer->fd ) ) {
        putsde ( "send failed" );
		close(peer->fd);
        return 0;
    }

    //waiting for response...

    if ( !acpp_readData ( output, &acpp_getFTSList, peer->fd ) ) {
        putsde ( "read failed \n" );
		close(peer->fd);
        return 0;
    }
//     if ( tl.length != 1 ) {
//         putsde ( "response: number of items = %d but 1 expected\n", tl.length );
//         return 0;
//     }
//     if ( tl.item[0].id != remote_channel_id ) {
//         putsde ( "response: peer returned id=%d but requested one was %d\n", tl.item[0].id, remote_channel_id );
//         return 0;
//     }
//     if ( tl.item[0].state != 1 ) {
//         putsde ( "response: FTS state is bad where remote_channel_id=%d\n", remote_channel_id );
//         return 0;
//     }
//     *output = tl.item[0];
close(peer->fd);
    return 1;
}

int acpp_peerSendI1ListCmd (char *cmd, I1List *input, Peer *peer ) {
    if ( !acpp_initClient ( peer, 3 ) ) {
        putsde ( "client not initialized" );
        return 0;
    }
    if ( !acpp_sendRequest ( cmd, input, &acpp_setI1List, peer->fd ) ) {
        putsde ( "send failed" );
		close(peer->fd);
        return 0;
    }
	close(peer->fd);
    return 1;
}

int acpp_peerSendI1F1ListCmd (char *cmd, I1F1List *input, Peer *peer ) {
    if ( !acpp_initClient ( peer, 3 ) ) {
        putsde ( "client not initialized" );
        return 0;
    }
    if ( !acpp_sendRequest ( cmd, input, &acpp_setI1F1List, peer->fd ) ) {
        putsde ( "send failed" );
		close(peer->fd);
        return 0;
    }
	close(peer->fd);
    return 1;
}

int acpp_FTSCat ( int id, double value, struct timespec tm, int state, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%ld" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, id, value, tm.tv_sec, tm.tv_nsec, state );
    return acpp_strCat ( buf, q );
}
int acpp_IDCat ( int id, double value, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR ACPP_FLOAT_FORMAT_OUT ACPP_DELIMITER_ROW_STR, id, value );
    return acpp_strCat ( buf, q );
}

int acpp_I2Cat ( int p0, int p1, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, p0, p1 );
    return acpp_strCat ( buf, q );
}

int acpp_I3Cat ( int p0, int p1, int p2, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, p0, p1, p2 );
    return acpp_strCat ( buf, q );
}

int acpp_I4Cat ( int p0, int p1, int p2, int p3, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, p0, p1, p2, p3 );
    return acpp_strCat ( buf, q );
}

int acpp_I5Cat ( int p0, int p1, int p2, int p3, int p4, char *buf ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_COLUMN_STR "%d" ACPP_DELIMITER_ROW_STR, p0, p1, p2, p3, p4 );
    return acpp_strCat ( buf, q );
}

