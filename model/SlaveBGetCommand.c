#include "SlaveGetCommand.h"

int sbgcList_init(SlaveBGetCommandList *list, const char *path ){
	RESET_LIST ( list )
	printf("broadcast get path: %s\n", path);
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, path ) ) {
        TSVclear ( db );
        return 1;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		putsdo("no slave broadcast get commands\n");
        return 1;
    }
    ALLOC_LIST ( list,nt );
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "failed to allocate memory for broadcast get list\n" );
        return 0;
    }
    for(int i = 0; i<nt; i++) {
		if(LL >= LML) break;
		int cmd = TSVgetis(db, i, "cmd");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("null returned while reading broadcast get file 2\n");
			return 0;
		}
		LIll.id = cmd;
		if ( !initMutex ( &LIll.mutex ) ) {
			FREE_LIST ( list );
			TSVclear ( db );
			putsde ( "failed to initialize slave data mutex\n" );
			return 0;
		}
		LIll.stread_success = NULL;
		LL++;
		//printf("init bpoll LL++\n");
    }
    //printf("bpoll LL=%d\n",LL);
    TSVclear ( db );
    return 1;
}

SlaveBGetCommand *sbgc_getByCmd(int cmd, SlaveBGetCommandList *list){
	FORLi{
		if(cmd == LIi.id){
		   return &LIi;
		}
	}
    return NULL;
}
