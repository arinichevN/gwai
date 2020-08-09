#include "SlaveSetCommand.h"

int sscList_init(SlaveSetCommandList *list, const char *dir, const char *file_name, const char *file_type){
    RESET_LIST ( list )
    char path[LINE_SIZE];
	path[0] = '\0';
	strncat(path, dir, LINE_SIZE - 1);
	strncat(path, file_name, LINE_SIZE - strlen(path) - 1);
	strncat(path, file_type, LINE_SIZE - strlen(path) - 1);
	printf("set path: %s\n", path);
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, path ) ) {
        TSVclear ( db );
        return 1;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		putsdo("\tno slave set commands\n");
        return 1;
    }
    ALLOC_LIST ( list,nt )
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "\tfailed to allocate memory for channel poll list\n" );
        return 0;
    }
    for(int i = 0;i<nt;i++){
		if(LL >= LML) break;
		int cmd = TSVgetis(db, i, "cmd");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("\tnull returned while reading channel_poll file 2\n");
			return 0;
		}
		LIll.id = cmd;
		LL++;
    }
    TSVclear ( db );
    return 1;
}
