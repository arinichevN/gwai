#!/bin/bash

APP=gwst
APP_DBG=`printf "%s_dbg" "$APP"`
INST_DIR=/usr/sbin
CONF_DIR=/etc/controller
CONF_DIR_APP=$CONF_DIR/$APP

#DEBUG_PARAM="-Wall -pedantic"
DEBUG_PARAM="-Wall -pedantic -g"
MODE_DEBUG=-DMODE_DEBUG
MODE_FULL=-DMODE_FULL

NONE=-DNONEANDNOTHING

function move_bin {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP $INST_DIR/$APP && \
	chmod a+x $INST_DIR/$APP && \
	chmod og-w $INST_DIR/$APP && \
	echo "Your $APP executable file: $INST_DIR/$APP";
}

function move_bin_dbg {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP_DBG $INST_DIR/$APP_DBG && \
	chmod a+x $INST_DIR/$APP_DBG && \
	chmod og-w $INST_DIR/$APP_DBG && \
	echo "Your $APP executable file for debugging: $INST_DIR/$APP_DBG";
}

function move_conf {
	([ -d $CONF_DIR ] || mkdir $CONF_DIR) && \
	([ -d $CONF_DIR_APP ] || mkdir $CONF_DIR_APP) && \
	cp -r ./config $CONF_DIR_APP && \
	chmod -R a+rw $CONF_DIR_APP
	echo "Your $APP configuration files are here: $CONF_DIR_APP";
}

#your application will run on OS startup
function conf_autostart {
	cp -v init.sh /etc/init.d/$APP && \
	chmod 755 /etc/init.d/$APP && \
	update-rc.d -f $APP remove && \
	update-rc.d $APP defaults 30 && \
	echo "Autostart configured";
}
#clear && clear &&  gcc -DMODE_DEBUG -c main.c -D_REENTRANT -Wall -pedantic -g
function build_acp {
	cd lib/ACP && \
	gcc $1 $3 -c ACP.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd command && \
	gcc $1 $3 -c util.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../ && \
	cd serial && \
	gcc $1 $3 -c ACPSerial.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSerialPortParam.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../TCP && \
	gcc $1 $3 -c ACPTCP.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../../../ && \
	ar -cqv libpac.a lib/ACP/ACP.o lib/ACP/command/util.o lib/ACP/serial/ACPSerial.o lib/ACP/serial/ACPSerialPortParam.o lib/ACP/TCP/ACPTCP.o
}
function build_acp_serial_client {
	cd lib/ACP/serial/client && \
	gcc $1 $3 -c ACPSCID.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSCIDLListm.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSCIDMapper.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSCPort.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSCPortLListm.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPSC.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../../../../ && \
	ar -cqv libpac.a lib/ACP/serial/client/ACPSCID.o lib/ACP/serial/client/ACPSCIDLListm.o lib/ACP/serial/client/ACPSCIDMapper.o lib/ACP/serial/client/ACPSCPort.o lib/ACP/serial/client/ACPSCPortLListm.o lib/ACP/serial/client/ACPSC.o
}
function build_acp_tcp_server {
	cd lib/ACP/TCP/server && \
	gcc $1 $3 -c ACPTSConnection.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPTSConnectionLList.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c ACPTS.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../../../../ && \
	ar -cqv libpac.a lib/ACP/TCP/server/ACPTSConnection.o  lib/ACP/TCP/server/ACPTSConnectionLList.o lib/ACP/TCP/server/ACPTS.o
}
function build_lib {
	cd lib  && \
	gcc $1 $3 -c app.c -D_REENTRANT $DEBUG_PARAM -lpthread && \
	gcc $1 $3 -c util.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c timef.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c serial.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c tsv.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../   && \
	ar -cqv libpac.a lib/app.o lib/util.o lib/timef.o lib/serial.o lib/tsv.o
}

function build_model {
	cd model  && \
	cd Channel
	gcc $1 $3 -c main.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c list.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../  && \
	cd SlaveGetCommand  && \
	gcc $1 $3 -c main.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../  && \
	cd SlaveIntervalGetCommand  && \
	gcc $1 $3 -c main.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../  && \
	cd ../   && \
	ar -cqv libpac.a model/Channel/main.o model/Channel/list.o model/SlaveGetCommand/main.o model/SlaveIntervalGetCommand/main.o
}

function build_app {
	cd app  && \
	gcc $1 $3 -c main.c -D_REENTRANT $DEBUG_PARAM -lpthread && \
	gcc $1 $3 -c print.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	gcc $1 $3 -c tcp_server.c -D_REENTRANT $DEBUG_PARAM -lpthread  && \
	cd ../   && \
	ar -cqv libpac.a app/main.o app/print.o app/tcp_server.o
}

#1				2
#debug_mode		bin_name
function build {
	#find . -maxdepth 16 -name '*.o' -type f -delete
	rm -f libpac.a
	build_lib $1 $2 $3 && \
	build_acp $1 $2 $3 && \
	build_acp_serial_client $1 $2 $3 && \
	build_acp_tcp_server $1 $2 $3 && \
	build_model $1 $2 $3 && \
	build_app $1 $2 $3 && \
	
	gcc  -D_REENTRANT $1 $3  main.c -o $2 $DEBUG_PARAM  -L./ -lpac -lpthread && \
	echo "Application" $2 "has been successfully compiled."
}

function part_debug {
	clear
	clear
	build $MODE_DEBUG $APP_DBG $NONE
}

function full {
	clear
	clear
	DEBUG_PARAM=$NONE
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL && \
	move_bin && move_bin_dbg && move_conf && conf_autostart
}

function full_nc {
	DEBUG_PARAM=$NONE
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL  && \
	move_bin && move_bin_dbg
}


function part {
	build $NONE $APP_DBG $NONE
}

function uninstall_nc {
	pkill $APP --signal 9
	pkill $APP_DBG --signal 9
	rm -f $INST_DIR/$APP
	rm -f $INST_DIR/$APP_DBG
}
function uninstall {
	uninstall_nc
	update-rc.d -f $APP remove
	rm -rf $CONF_DIR_APP
}


f=$1
${f}
