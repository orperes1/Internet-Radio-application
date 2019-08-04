#include "define.h"
#include "phy.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

extern int tcp_socket;
extern int state;
extern pthread_t control;

int main(int argc,char* argv[]) {

	char * server_name;
	uint16_t server_port;

	/* check the amount of arg */
	if (argc != 3){
		printf("Not enough arg (server name, port)\n");
		exit(0);
	}

	/* set the var*/
	server_name = argv[1];
	server_port = atoi(argv[2]);

	state = OFF;
	tcp_socket = open_tcp_sock(server_name,server_port);
	send_hello (tcp_socket);
	live = 1;
	pthread_create(&control,NULL,&control_thread,NULL);
	pthread_join (control,NULL);
	return 1;
}
