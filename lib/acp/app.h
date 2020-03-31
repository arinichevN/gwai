#ifndef LIBPAS_ACP_APP_H
#define LIBPAS_ACP_APP_H

				
#define SEND_STR(V) acptcp_send(V, fd);

#define CMD_IS(V) strncmp(cmd, V, ACP_CMD_MAX_LENGTH)==0

#define CDS ACP_DELIMITER_COLUMN_STR
#define RDS ACP_DELIMITER_ROW_STR

#endif
