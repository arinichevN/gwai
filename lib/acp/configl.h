#ifndef ACP_CONFIG_H
#define ACP_CONFIG_H

#include "data.h"

extern int configl_getPeerList(PeerList *list, const char *db_path);

extern int configl_getRChannel(RChannel *item, int rchannel_id, sqlite3 *dbl, const char *db_path);

extern int configl_getPeer(Peer *item, const char * peer_id, sqlite3 *db, const char *db_path);

extern int configl_getPort(int *port, const char *peer_id, sqlite3 *dbl, const char *db_path);

#endif 

