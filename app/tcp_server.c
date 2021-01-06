#include "tcp_server.h"

extern ACPSC *serial_client;
extern ChannelList channels;

static void sendRawDataToClient (char *data, int tcp_fd,  Mutex *mutex) {
	char q[ACP_BUF_MAX_LENGTH];
	mutex_lock(mutex);
	strncpy(q, data, ACP_BUF_MAX_LENGTH);
	mutex_unlock(mutex);
	acptcp_send(tcp_fd, q);
}

static int serveAppCommand(int tcp_fd, const char *serial_request_str, char sign) {
	int command;
	if (!acp_packGetCellI(serial_request_str, ACP_REQUEST_IND_CMD, &command)) {
		putsde("failed to get command\n");
		return 0;
	}
	switch(command){
		case CMD_GATEWAY_PRINT:
			{
			if(sign != ACP_SIGN_REQUEST_GET) return 0;
			int id;
			if (!acp_packGetCellI(serial_request_str, ACP_REQUEST_IND_ID, &id)) {
				putsde("failed to get ID\n");
				return 0;
			}
			if(id != APP_ID) {
				printde("my ID is %d, but %d found\n", APP_ID, id);
				return 0;
			}
			app_print(tcp_fd);}
			return 1;
	}
	return 0;
}

static int serveGetCommand(int tcp_fd, const char *serial_request_str) {
	int id;
	if (!acp_packGetCellI(serial_request_str, ACP_REQUEST_IND_ID, &id)) {
		putsde("failed to get ID\n");
		return 0;
	}
	Channel *channel = NULL;
	LIST_GETBYID(channel, &channels, id)
	if(channel != NULL){
		int command;
		if (!acp_packGetCellI(serial_request_str, ACP_REQUEST_IND_CMD, &command)) {
			putsde("failed to get command\n");
			return 0;
		}
		SlaveGetCommand *igcmd = channel_getIntervalGetCmd(channel, command);
		if(igcmd != NULL){
			if(igcmd->result == ACP_SUCCESS){
				sendRawDataToClient(igcmd->data, tcp_fd,  &igcmd->mutex);
				printdo("iget cmd %d for channel %d data %s\n", command, channel->id, igcmd->data);
				return 1;
			}else{
				printde("auto get command (%d) for channel id=%d: bad data\n", command, id);
			}
		}else{
			printde("auto get command (%d) not found for channel id=%d\n", command, id);
		}
	}
	size_t resp_len = ACP_BUF_MAX_LENGTH;
	char response[resp_len];
	memset(response, 0, resp_len);
	if(acpsc_getFromRemoteID(serial_client, id, serial_request_str, response, resp_len) != ACP_SUCCESS){
		return 0;
	}
	acptcp_convertSerialPack(response);
	acptcp_send(tcp_fd, response);
	return 1;
}

static int serveSetCommand(int tcp_fd, const char *serial_request_str) {
	int id;
	if (!acp_packGetCellI(serial_request_str, ACP_REQUEST_IND_ID, &id)) {
		putsde("failed to get ID\n");
		return 0;
	}
	if(acpsc_sendRequestToRemoteID(serial_client, id, serial_request_str) != ACP_SUCCESS){
		return 0;
	}
	return 1;
}

static int serveBroadcastSetCommand(int tcp_fd, const char *serial_request_str) {
	if(acpsc_sendRequestBroadcast(serial_client, serial_request_str) != ACP_SUCCESS){
		return 0;
	}
	return 1;
}

static int serveBroadcastGetCommand(int tcp_fd, const char *serial_request_str) {
	char response[ACP_BUF_MAX_LENGTH];
	if(acpsc_getBroadcast(serial_client, serial_request_str, response, ACP_BUF_MAX_LENGTH) != ACP_SUCCESS){
		return 0;
	}
	acptcp_convertSerialPack(response);
	acptcp_send(tcp_fd, response);
	return 1;
}

int serveTCPRequest(int tcp_fd, const char *tcp_request_str){
	char serial_request_str[ACP_BUF_MAX_LENGTH];
	strncpy(serial_request_str, tcp_request_str, ACP_BUF_MAX_LENGTH);
	if(!acpserial_tcpToSerial(serial_request_str, ACP_BUF_MAX_LENGTH)){
		putsde("failed to convert TCP package to serial\n");
		return 0;
	}
	char sign = serial_request_str[ACP_IND_SIGN];
	if(serveAppCommand(tcp_fd, serial_request_str, sign)){
		return 1;
	}
	switch(sign){
		case ACP_SIGN_REQUEST_GET:
			return serveGetCommand(tcp_fd, serial_request_str);
		case ACP_SIGN_REQUEST_SET:
			return serveSetCommand(tcp_fd, serial_request_str);
		case ACP_SIGN_REQUEST_SET_BROADCAST:
			return serveBroadcastSetCommand(tcp_fd, serial_request_str);
		case ACP_SIGN_REQUEST_GET_BROADCAST:
			return serveBroadcastGetCommand(tcp_fd, serial_request_str);
		default:
			putsde("bad sign\n");
			return 0;
	}
	putsdo("tcp: unknown command\n");
	return 0;
}
