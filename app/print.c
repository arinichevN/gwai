#include "print.h"

extern int sock_port;
extern int tcp_conn_num;

extern Acpts *tcp_server;
extern Acpsc *serial_client;
extern NoidList noids;

#define SEND_STR(V) acptcp_send(fd, V);

void app_print(int fd){
	char q[128];
	snprintf(q, sizeof q, "APP_ID: %d\n", APP_ID);
	SEND_STR(q)
	snprintf(q, sizeof q, "CONFIG_FILE: %s\n", CONFIG_FILE);
	SEND_STR(q)
	snprintf(q, sizeof q, "NOIDS_CONFIG_FILE: %s\n", NOIDS_CONFIG_FILE);
	SEND_STR(q)
	snprintf(q, sizeof q, "port: %d\n", sock_port);
	SEND_STR(q)
	snprintf(q, sizeof q, "tcp_conn_num: %d\n", tcp_conn_num);
	SEND_STR(q)


	SEND_STR("+----------------------------------------------------+\n")
	SEND_STR("|              serial client ports                   |\n")
	SEND_STR("+--------------+-------------+-----------+-----------+\n")
	SEND_STR("|      ptr     |  filename   |    DPS    |    rate   |\n")
	SEND_STR("+--------------+-------------+-----------+-----------+\n")
	FOREACH_LLIST(item, &serial_client->ports, AcpscPort) {
		snprintf(q, sizeof q, "|%14p|%13s|%11s|%11d|\n",
				   (void *)item,
				   item->param.filename,
				   item->param.dps,
				   item->param.rate
				);
		SEND_STR(q)
	}
	SEND_STR("+--------------+-------------+-----------+-----------+\n")



	SEND_STR("+--------------------------------------+\n")
	SEND_STR("|         serial client IDs            |\n")
	SEND_STR("+-----------+--------------+-----------+\n")
	SEND_STR("|    value  |    port_ptr  |   state   |\n")
	SEND_STR("+-----------+--------------+-----------+\n")
	FOREACH_LLIST(item, &serial_client->ids, AcpscID) {
		snprintf(q, sizeof q, "|%11d|%14p|%11s|\n",
				   item->value,
				   (void *)item->owner,
				   acpscid_getStateStr(item)
				);
		SEND_STR(q)
	}
	SEND_STR("+-----------+--------------+-----------+\n")



	SEND_STR("+--------------------------------------------------------------------------+\n")
	SEND_STR("|                                     noids                                |\n")
	SEND_STR("|           +--------------------------------------------------------------+\n")
	SEND_STR("|           |                           poll commands                      |\n")
	SEND_STR("+-----------+-----------+-----------+-----------+--------------+-----------+\n")
	SEND_STR("|     id    |    cmd    |interval_s |interval_ns|    result    |   state   |\n")
	SEND_STR("+-----------+-----------+-----------+-----------+--------------+-----------+\n")
	FORLISTN(noids, i){
		Noid *noid = &noids.item[i];
		FORLISTN(noid->igcmd_list, j){
			NoidIntervalGetCommand *item = &noid->igcmd_list.item[j];
			snprintf(q, sizeof q, "|%11d|%11d|%11ld|%11ld|%14s|%11s|\n",
						noid->id,
						item->command.id,
						item->interval.tv_sec,
						item->interval.tv_nsec,
						acp_getResultStr(item->command.result),
						nigc_getStateStr(item)
					);
			SEND_STR(q)
		}
	}
	SEND_STR("+-----------+-----------+-----------+-----------+--------------+-----------+\n")



	SEND_STR("+-----------------------+\n")
	SEND_STR("|    TCP connections    |\n")
	SEND_STR("+-----------+-----------+\n")
	SEND_STR("|     id    |   state   |\n")
	SEND_STR("+-----------+-----------+\n")
	FOREACH_LLIST(item, &tcp_server->connections, AcptsConnection) {
			snprintf(q, sizeof q, "|%11zu|%11s|\n",
						item->id,
						acptsconn_getStateStr(item)
					);
				SEND_STR(q)
	}
	SEND_STR("+-----------+-----------+\n")
	SEND_STR(">\n\0")
}

#undef SEND_STR
