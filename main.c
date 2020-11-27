#include "main.h"


static Serverm server;

static Mutex serial_thread_list_mutex = MUTEX_INITIALIZER;
static SerialThreadStarter serial_thread_starter;
static SerialThreadLList serial_thread_list = LLIST_INITIALIZER;
static ChannelList channel_list = LIST_INITIALIZER;


#include "model/SlaveGetCommand.c"
#include "model/SlaveSetCommand.c"
#include "model/SlaveIntervalGetCommand.c"
#include "model/SlaveBGetCommand.c"
#include "model/Channel.c"
#include "model/SerialThread.c"
#include "model/SerialThreadStarter.c"
#include "util.c"
#include "server.c"
#include "app.c"




int main ( int argc, char** argv ) {
#ifdef MODE_FULL
    daemon ( 0, 0 );
#endif
    conSig ( &app_exit );
    while ( 1 ) {
       //printdo ( "%s(): %s\n", F, getAppState() );
	   control();
    }
    putsde ( "unexpected while break\n" );
    return ( EXIT_FAILURE );
}
