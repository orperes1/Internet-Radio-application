#include "define.h"
#include "phys.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

extern int state, clients_num;
extern int tcp_port;
extern uint16_t udp_port;
extern struct UDP_info * station_list;
extern struct TCP_info * client_list , *last_client ;

int main(int argc, char* argv[]) {
	char user_command , *temp_song_name;
	int welcome_sock, inputfd,error ,s_num, i;
	UDP_info *udp_station, *udp_point;
	TCP_info *new_client;
	struct sockaddr_in server_sock;
	socklen_t len;
	fd_set fdset;

	station_list = NULL;
	client_list = NULL;
	last_client = NULL ;
	clients_num = 0;
	/* set the var*/
	tcp_port = atoi(argv[1]);
	memcpy(mcast_ip,argv[2],strlen(argv[2]));
	udp_port = atoi(argv[3]);
	/* check the amount of arg */
	if (argc < 5) {
		printf("Not enough arg (port, mcast ip , mcust port ,song name \n");
		exit(0);
	}

	state = S_OFF;
	/*opening the client songs dir */
	if ((opendir("songs_from_clients")==NULL) && (mkdir("songs_from_clients",(__mode_t)0700)==-1)) {
		perror("can't open dir\n");
		exit(0);
	}

	/*open UDP sockets for all stations*/
	s_num = argc - 4;
	for (i = 0; i < s_num; i++) {
		udp_station = (UDP_info*) malloc(sizeof(struct UDP_info));
		temp_song_name = (char*)argv[i+4];
		open_udp_socket(udp_station,temp_song_name,mcast_ip,udp_port,i);
		if (i == 0){ //changing pointers
			station_list =  udp_station;
			udp_point = station_list;
		} else{
			udp_point->next = udp_station;
			udp_point = udp_point->next;
		}
		last_station = udp_station ;
		pthread_create(&udp_station->station_th, NULL, &play_th, (void*)i);
	}

	/*open welcome socket - tcp*/
	welcome_sock = open_tcp_sock(INADDR_ANY, tcp_port);
	error = listen(welcome_sock,1);
	if (error < 0){
		perror("Failed to listen\n");
		closing_socket(welcome_sock);
		Terminate(ERR);
	}

	state = S_ESTABLISH;
	len = sizeof(server_sock);

	printf("Please type ""q"" to quit or ""p"" to print the status\n");
	while (TRUE) {
		/*define set of file descriptors*/
		FD_ZERO(&fdset);
		FD_SET(welcome_sock, &fdset);
		FD_SET(fileno(stdin), &fdset);

		/*for each interput - enter to specific function*/
		inputfd = select(welcome_sock+1, &fdset, NULL, NULL, NULL);
		if (inputfd == -1) {
			printf("Error while select function!\n");
			Terminate(ERR);
		} else if (FD_ISSET(fileno(stdin), &fdset)) { // Interrupt from user
			fflush(stdin);
			scanf("%c", &user_command);

			switch (user_command) {
			case ('P'):
			case ('p'): {
				print_data();
				break;
			}
			case ('Q'):
			case ('q'): {
				Terminate(LEAVE);
				break;
			}
			default: {
				printf("Invalid command\n");
				break;
			}
			}
			printf("Please type ""q"" to quit or ""p"" to print the status\n");
		} else if (FD_ISSET(welcome_sock, &fdset)) {// new client
			printf ("New client arrived\n");
			if (clients_num > 100){
				printf("There is no place to more clients! sorry..\n");
				break;
			}
			new_client = (struct TCP_info*) malloc(sizeof(struct TCP_info));
			new_client->socket = accept(welcome_sock,(struct sockaddr*) &server_sock,&len);

			if(new_client->socket == -1){
				perror("Error to open new socket\n");
				free(new_client);
			}
			else {
				//update pointers
				if(clients_num == 0){
					last_client = new_client;
					client_list = new_client;
				} else{
					last_client->next = new_client;
					last_client =new_client;
				}
				new_client->next = NULL;
				clients_num++;
				new_client_add(new_client,server_sock);
				pthread_create(&new_client->client_th,NULL,&client_handler,(void*)new_client);
			}
		}
	}
	return 1;
}
