
#include "../dbl.h"
#include "main.h"
#include "configl.h"
#include "../dstructure_auto.h"
#include "data.h"

int configl_checkPeerList ( const PeerList *list ) {
    //unique id
    for ( int i = 0; i < list->length; i++ ) {
        for ( int j = i + 1; j < list->length; j++ ) {
            if ( strcmp ( list->item[i].id, list->item[j].id ) == 0 ) {
                fprintf ( stderr, "checkPeerList: id = %s is not unique\n", list->item[i].id );
                return 0;
            }
        }
    }
    return 1;
}


static int getPeerList_callback ( void *data, int argc, char **argv, char **azColName ) {
    PeerList *list = data;
    int c = 0;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            if ( strlen ( DB_COLUMN_VALUE ) >= ACP_PEER_ID_LENGTH ) {
                printde ( "id length exceded (no more %d expected)\n", ACP_PEER_ID_LENGTH-1 );
                return EXIT_FAILURE;
            }
            strncpy ( list->item[list->length].id, DB_COLUMN_VALUE,ACP_PEER_ID_LENGTH );
            c++;
        } else if ( DB_COLUMN_IS ( "port" ) ) {
            list->item[list->length].port = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "ip_addr" ) ) {
            if ( strlen ( DB_COLUMN_VALUE ) >= ACP_PEER_ADDR_STR_LENGTH ) {
                printde ( "id length exceded (no more %d expected)\n", ACP_PEER_ADDR_STR_LENGTH-1 );
                return EXIT_FAILURE;
            }
            strncpy ( list->item[list->length].addr_str, DB_COLUMN_VALUE,ACP_PEER_ADDR_STR_LENGTH );
            c++;
        } else {
            putsde ( "unknown column\n" );
        }
    }
#define N 3
    if ( c != N ) {
        printde ( "required %d columns but %d found\n", N, c );
        return EXIT_FAILURE;
    }

#undef N
    list->length++;
    return EXIT_SUCCESS;
}

static int getRChannelList_callback ( void *data, int argc, char **argv, char **azColName ) {
    struct ds {
        void *p1;
        void *p2;
    } *d;
    d=data;
    sqlite3 *db=d->p2;
    RChannelList *list = d->p1;
    int c = 0;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            list->item[list->length].id = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "peer_id" ) ) {
            if ( !configl_getPeer ( &list->item[list->length].peer, DB_COLUMN_VALUE, db, NULL ) ) {
                printde ( "peer not found by its id=%s\n",  DB_COLUMN_VALUE );
                return EXIT_FAILURE;
            }
            c++;
        } else if ( DB_COLUMN_IS ( "channel_id" ) ) {
            list->item[list->length].channel_id = DB_CVI;
            c++;
        } else {
            putsde ( "unknown column\n" );
        }
    }
#define N 3
    if ( c != N ) {
        printde ( "required %d columns but %d found\n", N, c );
        return EXIT_FAILURE;
    }
#undef N
    list->length++;
    return EXIT_SUCCESS;
}

int configl_getPeerList ( PeerList *list, const char *db_path ) {
    RESET_LIST ( list )
    sqlite3 *db;
    if ( !db_openR ( db_path, &db ) ) {
        putsde ( "failed\n" );
        return 0;
    }
    int n = 0;
    char *qn = "select count(*) FROM peer";
    db_getInt ( &n, db, qn );
    if ( n <= 0 ) {
        db_close ( db );
        return 1;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
        putsde ( "failed to allocate memory\n" );
        db_close ( db );
        return 0;
    }
    char *q = "select id, port, ip_addr FROM peer";
    if ( !db_exec ( db, q, getPeerList_callback, list ) ) {
        putsde ( "failed\n" );
        db_close ( db );
        FREE_LIST ( list );
        return 0;
    }
    db_close ( db );
    if ( !configl_checkPeerList ( list ) ) {
        FREE_LIST ( list );
        return 0;
    }
    return 1;
}

int configl_getPort ( int *port, const char *peer_id, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openRAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    char q[LINE_SIZE];
#define CONFIGL_BAD_PORT -1
    int _port=CONFIGL_BAD_PORT;
    snprintf ( q, sizeof q, "SELECT port FROM peer where id='%s'", peer_id );
    if ( !db_getInt ( &_port, db, q ) ) {
        printde ( "failed where peer_id=%s\n", peer_id );
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    if ( _port==CONFIGL_BAD_PORT ){
        printde ( "not found where peer_id=%s\n", peer_id );
        return 0;
    }
    *port=_port;
    return 1;
}


int configl_getRChannel ( RChannel *item, int rchannel_id, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openRAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    char q[LINE_SIZE];
    RChannelList l = {.item = item, .length = 0, .max_length = 1};
    struct ds {
        void *p1;
        void *p2;
    } data= {.p1=&l, .p2=db};
    memset ( item, 0, sizeof *item );
    snprintf ( q, sizeof q, "SELECT id, peer_id, channel_id FROM remote_channel where id=%d", rchannel_id );
    if ( !db_exec ( db, q, getRChannelList_callback, &data ) ) {
        putsde ( "failed\n" );
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    if ( l.length != 1 ) {
        printde ( "can't get remote channel: %d\n", rchannel_id );
        return 0;
    }

    return 1;
}

int configl_getPeer ( Peer *item, const char * peer_id, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openRAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    char q[LINE_SIZE];
    PeerList data = {.item = item, .length = 0, .max_length = 1};
    memset ( item, 0, sizeof *item );
    snprintf ( q, sizeof q, "SELECT id, port, ip_addr FROM peer where id='%s'", peer_id );
    if ( !db_exec ( db, q, getPeerList_callback, &data ) ) {
        putsde ( "failed\n" );
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    return 1;
}
