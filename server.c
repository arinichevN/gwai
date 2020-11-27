
void app_reset();

void sendRawDataToClient (char *data, int tcp_fd,  Mutex *mutex ) {
	char q[ACP_BUF_MAX_LENGTH];
	lockMutex(mutex);
	strncpy(q,data,ACP_BUF_MAX_LENGTH);
	unlockMutex(mutex);
	acptcp_send ( q, tcp_fd );
}

int serveChannelCmd(int SERVER_FD, int SERVER_CMD, char *buf){
	printdo("serving request for channel: %s\n", buf);
	int channel_id;
	if ( !acp_packGetCellInt(buf, ACP_REQUEST_IND_ID, &channel_id) ) {
		putsde("failed to get channel id\n");
		return 0;
	}
	printdo("tcp: channel_id: %d\n", channel_id);
	Channel *channel = NULL;
	LIST_GETBYID(channel, &channel_list, channel_id)
	if(channel == NULL || channel->thread == NULL){
		putsde("no channel or channel thread\n");
		return 0;
	 }
	lockMutex(&channel->mutex);
	if(channel->thread == NULL) goto failed;
	
	char sign = buf[ACP_IND_SIGN];
	if(sign == ACP_SIGN_REQUEST_GET){
		SlaveGetCommand *igcmd = channel_getIntervalGetCmd(channel, SERVER_CMD);
		if(igcmd != NULL){
			//printdo("iget cmd %s for channel %d data %s\n", SERVER_CMD, channel->id, igcmd->data);
			if( igcmd->result == ACP_SUCCESS ){
				sendRawDataToClient(igcmd->data, SERVER_FD,  &igcmd->mutex);
				goto success;
			}
		}else{
			//printdo("get cmd %s for channel %d\n", SERVER_CMD, channel->id);
			channel_slaveToClient(channel, buf, SERVER_FD );
			goto success;
		}
	}else if(sign == ACP_SIGN_REQUEST_SET){
		//printdo("set cmd %s for channel %d\n", SERVER_CMD, channel->id);
		channel_sendRawDataToSlave(channel, buf);
		goto success;
	}

	failed:
	unlockMutex(&channel->mutex);
	putsdo("request not served\n");
	return 0;
	success:
	unlockMutex(&channel->mutex);
	putsdo("request successfully served\n");
	return 1;
}

int serveAppCmd(int SERVER_FD, int SERVER_CMD, char *buf){
	switch(SERVER_CMD){
		case CMD_GATEWAY_PRINT:
			printData ( SERVER_FD );
	        return 1;
	    case CMD_GATEWAY_RESET:
		    app_reset();
			return 1;
		default:
			putsdo("no app command matched\n");
	}
    return 0;
}



SlaveSetCommand *getSetCmd(int cmd, SlaveSetCommandList *list){
	FORLi{
		if(cmd == LIi.id){
		   return &LIi;
		}
	}
    return NULL;
}

int sendRawDataToSlaveBroadcast (char *pack_str, SerialThreadLList *list ) {
	FORLLi(SerialThread){
		Mutex *mutex = &item->mutex;
		int fd = item->fd;
		lockMutex(mutex);
		acpserial_sendTcpPack(fd, pack_str);
		unlockMutex(mutex);
	}
	return 1;
}

int slaveToClientBroadcast (char *pack_str, int tcp_fd, SerialThreadLList *list ) {
	FORLLi(SerialThread){
		Mutex *mutex = &item->mutex;
		int fd = item->fd;
		//sending request to slave
		lockMutex(mutex);
		putsdo("broadcast : send to serial\n");
		int r = acpserial_sendTcpPack(fd, pack_str);
		if(r != ACP_SUCCESS){
			unlockMutex(mutex);
			putsde("failed to send\n");
			continue;
		}
		//reading slave response
		size_t resp_len = ACP_BUF_MAX_LENGTH;
		char response[resp_len];
	    memset(response, 0, resp_len);
	    SRV_SLEEP_BEFORE_READ_SLAVE
		r = acpserial_readResponse(fd, response, resp_len);
		unlockMutex(mutex);
		if(r != ACP_SUCCESS){
			putsde("communication error while reading slave response\n");
			continue;
		}
		r = acpserial_checkCRC(response);
		if(r != ACP_SUCCESS){
			putsde("bad response crc\n");
			continue;
		}
		acptcp_convertSerialPack(response);
		//sending slave response to client
		acptcp_send ( response, tcp_fd );
		return 1;
	}
	putsde("failed to get broadcast data\n");
	return 0;
}

int serveBroadcastCmd(int SERVER_FD, int SERVER_CMD, char *buf){
	char sign = buf[ACP_IND_SIGN];
	if(sign == ACP_SIGN_REQUEST_GET_BROADCAST){
		slaveToClientBroadcast (buf, SERVER_FD, &serial_thread_list);
		goto success;
	}else if(sign == ACP_SIGN_REQUEST_SET_BROADCAST){
		sendRawDataToSlaveBroadcast(buf, &serial_thread_list);
		goto success;
	}
	//putsdo("broadcast command not found\n");
	return 0;
	success:
	//putsdo("broadcast command served\n");
	return 1;
}

int serveRequest(int SERVER_FD, char *buf){
	int SERVER_CMD;
	if ( !acp_packGetCellInt(buf, ACP_REQUEST_IND_CMD, &SERVER_CMD)) {
		putsde("failed to get command\n");
		return 0;
	}
	printdo("tcp: command: %d\n", SERVER_CMD);
	if(serveBroadcastCmd(SERVER_FD, SERVER_CMD, buf)){
		return 1;
	}else if(serveChannelCmd(SERVER_FD, SERVER_CMD, buf)){
		return 1;
	}else if (serveAppCmd(SERVER_FD, SERVER_CMD, buf) ) {
		return 1;
	}
	printdo("tcp: unknown command: %d\n", SERVER_CMD);
	return 0;
}
