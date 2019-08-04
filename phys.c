#include "phys.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <math.h>

int open_tcp_sock(in_addr_t server_name, uint16_t server_port) {
	/* the function receive the server info and open a tcp socket to the server */
	int error, tcp_socket, one;
	struct sockaddr_in reciver;
	state = OFF;
	/*open a socket and set its properties*/
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("Failed to open socket\n");
		Terminate(ERR);
	}

	/* set the sender info */
	reciver.sin_family = AF_INET;
	reciver.sin_port = htons(server_port);
	reciver.sin_addr.s_addr = htonl(server_name);
	memset(reciver.sin_zero, '\0', sizeof reciver.sin_zero);

	/*reuse if sock*/
	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &one ,sizeof(one)) < 0){
		perror("setsockopt(SO_REUSEPORT) failed");
		closing_socket(tcp_socket);
	}

	/* bind */
	error = bind(tcp_socket, (struct sockaddr *) &reciver, sizeof(reciver));
	if (error < 0) {
		perror("Failed to connect to server \n");
		Terminate(ERR);
	}

	return tcp_socket;
}

void open_udp_socket(UDP_info *udp_station, char *song_name, char *ip_ad,int udp_port,int num) {
	/* the function fill the strut open the file and the udp sock */
	int error,one;
	const char ttl = TTL;

	strcpy(udp_station->song_name,song_name); // enter the song name to the the struct
	udp_station->socket= socket(AF_INET,SOCK_DGRAM,0);// opening sock
	if (udp_station->socket < 0){
		perror("Failed to open socket\n");
		Terminate(ERR);
	}

	/* set the sender info */
	udp_station->ip_addr.sin_family =  AF_INET;
	udp_station->ip_addr.sin_port = htons(udp_port);
	udp_station->ip_addr.sin_addr.s_addr = inet_addr(ip_ad);
	udp_station->ip_addr.sin_addr.s_addr = ntohl(htonl(udp_station->ip_addr.sin_addr.s_addr)+num);
	memset(udp_station->ip_addr.sin_zero, '0', sizeof udp_station->ip_addr.sin_zero);

	/*enable reuse of sockets*/
	if (setsockopt(udp_station->socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &one ,sizeof(one)) < 0)
		    perror("setsockopt(SO_REUSEPORT) failed");


	/*bind*/
	error = bind(udp_station->socket,(struct sockaddr *) &udp_station->ip_addr, sizeof(udp_station->ip_addr));
	if (error < 0 ){
		perror("Failed to bind2\n");
		Terminate(ERR);
	}

	/* mcust grope */
	error= setsockopt(udp_station->socket,IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (error < 0 ){
		perror("Failed to set socket opt\n");
		Terminate(ERR);
	}

	/* open the song */
	memcpy(udp_station->song_name,song_name,strlen(song_name));
	udp_station ->fp = fopen(song_name,"r");
	if (udp_station ->fp == NULL){
		printf("song_name %s\n",song_name);
		perror("can't open song file \n");
		Terminate(ERR);
	}

	/* set the stuct var */
	udp_station->live = 1;
	udp_station->from_user = 0;
	udp_station->num_station = max_stations_num;
	max_stations_num ++;
	udp_station->next = NULL;

}

void * play_th(void * station_num ) {
	/* this function is for the udp sock - the function play the station song reputedly */
	int station_index = (int)station_num;
	UDP_info * station = station_list;
	FILE * song;
	int error,i;
	char buffer[BUFFER_SIZE];

	for(i=0;i<station_index;i++){// finding the correct station
		station = station->next;
	}

	song = station->fp;
	while (station->live == 1) {
		error = fscanf(song,"%1024c", buffer);
		if(error ==EOF){// check if the the EOF and rewind
			rewind(song);
		}
		error = sendto(station->socket,buffer,BUFFER_SIZE,0,(struct sockaddr *) &station->ip_addr, sizeof(station->ip_addr));
		if (error < 0 ) {
			perror("Failed to send file\n");
			Terminate(ERR);
		}
		usleep(OPT_RATE);// set the transmiting rate
	}/* end of while */
	return (void*)1;
}

void Terminate(int error_type) {
	/* this function is entailing all the sever shut done */
	int i = 0;
	UDP_info * station_temp = station_list;
	TCP_info * client_temp = client_list ;
	printf("SHUTING DOWN\n");
	if (error_type == LEAVE){
		printf("User ask to leave :(\n");
	}

	/* deleting stations*/
	for (i=0;i< max_stations_num; i++){
		if (fclose(station_temp->fp)){
			perror("Fail to close song file\n");
		}
		station_temp->live = 0 ;
		sleep (0.2);
		closing_socket(station_temp->socket);
		station_temp = station_temp->next;
		free(station_list);
		station_list = station_temp;
	}
	printf("Finish to remove all the stations\n");
	/* deleting clients*/
	for (i=0;i< clients_num; i++){
		client_temp->live = 0 ;
		station_temp = (UDP_info *) (client_temp->next);
		station_list = (UDP_info *)client_temp;
	}
	system("rm -r ./songs_from_clients");
	printf("Finish to remove all the clients \nGOOD BYE \n");
	sleep(1);
	exit(1);
}
void print_data() {
	/*the function print the server stat*/
	int i ,prefix = 0;
	UDP_info *  station_runer = station_list;
	TCP_info * client_runer = client_list;

	/*print the client details*/
	if (clients_num == 0)// if te client list is empty
		printf("There is no clients right now \n");
	else
		printf("The client list is:\n");

	for (i=0;i<clients_num;i++){
		printf("Client %d is: %s\n",i,client_runer->ip_addr);
		client_runer =client_runer->next;
	}

	/*print the station details*/
	printf("The station list:\n");
	for (i=0;i<max_stations_num;i++){
		if (station_runer->from_user)
			prefix = PREFIX_LEN;
		printf("Station num: %d song name is: %s \n",station_runer->num_station,station_runer->song_name+prefix);
		station_runer =station_runer->next;
		prefix = 0;
	}
}

void new_client_add(TCP_info * new_client,struct sockaddr_in server){
	/* the function enter values to the tcp struck */
	struct sockaddr_in *temp = (struct sockaddr_in*)&server;
	struct in_addr ip_addr = temp->sin_addr;

	new_client->client_sock.sin_family = AF_INET;
	new_client->client_sock.sin_port = htonl((uint16_t)tcp_port);
	new_client->client_sock.sin_addr.s_addr = ip_addr.s_addr;
	inet_ntop(AF_INET,&ip_addr.s_addr,new_client->ip_addr,INET_ADDRSTRLEN);
	new_client->live = 1 ;
}

void * client_handler(void *new_client){
	/*  this function responsible to the connection with the clients */
	TCP_info * sock_data = (TCP_info*)new_client;
	char buffer[BUFFER_SIZE];
	char msg_type;
	int error,i,inputfd;
	fd_set fdset;
	struct timeval timeout;
	pthread_t tid = sock_data->client_th;

	/* at the first connection we need to wait to the hello msg or wait for timeout */
	//define set of file descriptors
	FD_ZERO(&fdset);
	FD_SET(sock_data->socket, &fdset);
	timeout.tv_sec = 0;
	timeout.tv_usec = TIMEOUT1 ;
	inputfd = select(sock_data->socket + 1, &fdset, NULL, NULL, &timeout);

	if (inputfd == -1){
		printf("Error at select function! - client_handler\n");
		Terminate(ERR);
	}
	else if(inputfd == 0){ // we didn't get the hello msg
		delete_client(sock_data,ERR_TIMEOUT);
	}
	else if(FD_ISSET(sock_data->socket,&fdset)){ //accept first hello msg  -send welcome
		error = recv(sock_data->socket,&buffer,BUFFER_SIZE,0);
		if(error<=0){
			delete_client(sock_data,error);
		}
		/* reed the hello msg */
		Hello client_hello;
		client_hello.commandType = buffer[1];
		client_hello.reserved = buffer[2]<<8 || buffer[3];
		if (client_hello.commandType != 0 ||client_hello.reserved != 0){ // check if the msg is valid
			send_invalid_msg(HELLO_ERROR,sock_data);
			delete_client(sock_data,error);
			return(void *)1;
		}
		send_welcome_msg(sock_data);
	}

	while(sock_data->live == 1){ // waiting for client massages
		error = recv(sock_data->socket,&buffer,BUFFER_SIZE,0);
		if(error<=0){
			delete_client(sock_data,error);
			break;
		}
		msg_type = buffer[0];
		switch (msg_type){
			case (HELLO):{ // Receiving anther hello msg
				send_invalid_msg(DUP_HELLO_ERROR,sock_data);
				delete_client(sock_data,error);
				return(void *)1;
			}
			case (ASKSONG):{ // geting a ask song msg
				AskSong client_AskSong;
				client_AskSong.commandType = ASKSONG;
				client_AskSong.stationNumber = buffer[1] <<8 | buffer[2];
				station_to_name(client_AskSong.stationNumber,sock_data); // convert the station number into a name of the song and sent the nsme
				break;
			}
			case (UPSONG):{ // user ask to uploading a song
				UpSong client_UpSong;
				client_UpSong.commandType = UPSONG;
				client_UpSong.songSize = ntohl(*(uint32_t*)&buffer[1]);
				client_UpSong.songNameSize = buffer[5];
				if(client_UpSong.songNameSize > SONG_NAME_SIZE){//check if the song name size is in the allowed boundaries
					printf("User tried to upload with name too long\n");
					send_permit(0,client_UpSong,sock_data);
				}
				for (i=0;i<client_UpSong.songNameSize;i++){// copy the song name to the struct
					client_UpSong.songName[i] = buffer[6+i];
				}
				if ( send_permit(1,client_UpSong,sock_data) ){ // sending the information to the send permit if the user can upload a song start  downloading
					state = S_UP_SONG;
					download_song(client_UpSong,sock_data);
				}

				break;

			}default: {// if we didn't get one of the know massages we send invalid msg
				send_invalid_msg(INVALID,sock_data);
				delete_client(sock_data,error);
				return(void *)1;
			}
		}
	}
	/* closing the thread */
	closing_socket(sock_data->socket);
	free(sock_data);
	pthread_exit (&tid);
}

void send_welcome_msg(TCP_info *sock_data){
	/* this function send welcome msg */
	Welcome server_Welcome;
	uint8_t buffer[9]={0};
	int error;

	/*fill the struct info */
	server_Welcome.replyType  = WELCOME;
	server_Welcome.numStations = max_stations_num;
	server_Welcome.multicastGroup = inet_addr(mcast_ip);
	server_Welcome.portNumber = udp_port;

	buffer[0] = server_Welcome.replyType;
	*(uint16_t*)&buffer[1] = htons(server_Welcome.numStations);
	*(uint32_t*)&buffer[3] = htonl(server_Welcome.multicastGroup );
	*(uint16_t*)&buffer[7] = htons(server_Welcome.portNumber);

	error = send(sock_data->socket,&buffer,WELCOME_LEN,0);
	if (error <= 0 ) {
		perror("Failed to send file \n");
		delete_client(sock_data,error);
	}
}

void station_to_name(uint16_t station_num,TCP_info *sock_data){
	/*this function find the song name and sent it to client - announce*/
	int i,error,song_len ,prefix =0;
	UDP_info * temp = station_list;
	char buffer[BUFFER_SIZE];

	if(station_num > max_stations_num){ //check if the client ask for wrong station num
		send_invalid_msg((int)WRONG_ST,sock_data);
		delete_client(sock_data,-1);
	}

	for (i=0;i<station_num;i++) // finding the relevant station
		temp = temp->next;

	if ( temp->from_user) // if the station play song that user uploaded we need to cut the song name
		prefix = PREFIX_LEN;

	song_len = strlen(temp->song_name)-prefix;

	/*send announce msg*/
	buffer[0] = ANNOUNE; //msg type
	buffer[1] = song_len; //songNameSize

	for(i=0;i<song_len;i++){ //copy the song name to the buffer
		buffer[2+i] = temp->song_name[i+prefix];
	}

	error = send(sock_data->socket,&buffer,2+strlen(temp->song_name),0);
	if (error <= 0 ) {
		perror("Failed to send file \n");
		delete_client(sock_data,error);
	}
}

int send_permit(int respond ,UpSong client_UpSong,TCP_info *sock_data){
	/* the function decide if the client has permission to uploading a song and send the permeation */
	char buffer[2];
	int error;
	buffer[0] = PERMITSONG;
	buffer[1] = 1 ;

	if (respond==0) // if the songe name is too long there is no permeation at the respond will be 0
		buffer[1] = 0 ;


	if( (client_UpSong.songSize<2000) || (client_UpSong.songSize>10*MEGA) ){// check the file size
		printf("Client try to send too big song !\n");
		buffer[1] = 0 ;
	}
	else if (search_song(client_UpSong.songName)){ // check if the song exist
		printf("Client try to send existing song!\n");
		buffer[1] = 0 ;
	}
	else if (state == S_UP_SONG){ // check if someone else is uploading a song
		printf("client try to send a song while the download is in process\n");
		buffer[1] = 0 ;
	}
	/*sending the msg*/
	error = send(sock_data->socket,&buffer,PERMITSONG_LEN,0);
	if (error <= 0 ) {
		perror("Failed to send file \n");
		delete_client(sock_data,error);
	}
	return buffer[1];// return if there is/isn't permeations
}

void download_song(UpSong client_UpSong,TCP_info *sock_data){
	// this function downloading the song from the client
	int error,size = 0,inputfd;
	char buffer[BUFFER_SIZE],song_n[SONG_DIR_SIZE];
	FILE * new_song;
	fd_set fdset;
	struct timeval timeout;

	sprintf(song_n,"songs_from_clients//%s",client_UpSong.songName);//change the song name to a dir/name
	sprintf(client_UpSong.songName,"%s",song_n);

	new_song = fopen(song_n,"w+");

	/* set the fd to the select*/
	FD_ZERO(&fdset);
	FD_SET(sock_data->socket, &fdset);
	timeout.tv_sec = TIMEOUT3;
	timeout.tv_usec = 0 ;

	//get the song
	while(TRUE){
		inputfd = select(sock_data->socket + 1, &fdset, NULL, NULL, &timeout);
		if (inputfd == -1){
			printf("Error at select function! -download_song \n");
			Terminate(ERR);
		}
		else if(inputfd == 0){ //timeout
			if (size != client_UpSong.songSize){ // check the receiving size
				printf("The expected file size is:%d , but the receiving file size is %d",client_UpSong.songSize,size);
				if (fclose(new_song)){
					perror("can't close file\n");
					Terminate(ERR);
				}
				if (remove(song_n)){
					perror("can't remove file\n");
					Terminate(ERR);
				}
				send_invalid_msg(UP_TIMEOUT,sock_data);
				state = S_ESTABLISH;
			}
			break;
		}
		else if(FD_ISSET(sock_data->socket,&fdset)){
			timeout.tv_sec = TIMEOUT3;
			timeout.tv_usec = 0 ;
			error = recv(sock_data->socket,buffer, BUFFER_SIZE,0);
			fwrite(buffer,sizeof(char),error,new_song);
			memset(buffer, '\0', sizeof(buffer));
			size+=error;
			if(size == client_UpSong.songSize){
				if (fclose(new_song)){
					perror("can't close file\n");
					Terminate(ERR);
				}
				send_new_station(client_UpSong.songName);//send new_station to all clients
			}
			else if (error == 0){
				delete_client(sock_data,error);
				break;
			}
		}
	}
}

void send_new_station(char * song_name){
	/* the function create new udp struct and sock and alert all the client about the new station */
	UDP_info *udp_station;
	NewStations new_st;
	uint8_t buffer [3];
	TCP_info * temp = client_list;
	int i,error, index;

	/*creating the satation */
	udp_station = (UDP_info*) malloc(sizeof(struct UDP_info));

	open_udp_socket(udp_station,song_name,mcast_ip,udp_port,max_stations_num);
	udp_station->from_user = 1;
	last_station ->next = udp_station;
	last_station = udp_station;
	index = max_stations_num -1;
	pthread_create(&udp_station->station_th, NULL, &play_th, (void*)index);

	new_st.replyType = NEWSTATIONS;
	new_st.newStationNumber = max_stations_num;
	buffer[0] = new_st.replyType;
	*(uint16_t*)&buffer[1] =htons(new_st.newStationNumber);

	/* sending info to all clients */
	for (i=0; i<clients_num ;i++){
		error = send(temp->socket,buffer,NEWSTATIONS_LEN,0);
		if (error <= 0 ) {
			perror("Failed to send file \n");
			delete_client(temp,error);
		}
		temp = temp->next;
	}
	printf("New station add\n");
	state = S_ESTABLISH;
}

void send_invalid_msg(int error_type, TCP_info * sock){
	/* the function send invalid msg to the client */
	int error;
	char buffer [BUFFER_SIZE],*replyString;
	buffer[0] = INVALIDCOMMAND;

	/* send the reason to the invalid msg*/
	switch(error_type){
		case (HELLO_ERROR):{
			sprintf(replyString,"Client didn't send hello msg\n");
			break;
		}
		case (DUP_HELLO_ERROR):{
			sprintf(replyString,"Client send more then one hello msg\n");
			break;
		}case (INVALID):{
			sprintf(replyString,"Client send unexpected msg\n");
			break;
		}case (WRONG_SIZE):{
			sprintf(replyString,"Client send msg with wrong size\n");
			break;
		}case (WRONG_ST):{
			sprintf(replyString,"Client ask for a song with wrong station num\n");
			break;
		}case (UP_TIMEOUT):{
			sprintf(replyString,"Timeout while uploading song\n");
			break;
		}default:{
			printf("BUG : wrong error_type at send_invalid_msg\n");
			break;
		}
	}
	buffer[1] =strlen(replyString);
	memcpy(buffer+2,&replyString,strlen(replyString));

	error = send(sock->socket,&buffer,strlen(replyString)+2,0);
	if (error <= 0 ) {
		perror("Failed to send file \n");
	}
}



void delete_client(TCP_info* client_tcp, int error){
	//close socket and delete client
	//-1 :error with receive from client
	//0: client close the connection /  client send invalid msg
	int i,flag = 0, delete_first=0;
	TCP_info * temp = client_list, *prev_client= NULL;

	/* check the reason for the delete*/
	switch (error){
		case(-1) :
				printf("Problem while communicate with client\n");
				break;
		case(0):
				printf("client leave the radio or sent invalid massage\n");
				break;
		case(ERR_TIMEOUT) :
				printf("client didn't send hello msg! TIMEOUT occur\n");
				break;
	}

	/* find the relevant client */
	for (i=0;i<clients_num;i++){
		if (temp == client_tcp){
			if (i == 0){
				delete_first = 1;
			}
			flag = 1;
			break;
		}
		prev_client = temp;
		temp = temp->next;
	}
	if (temp->next == NULL)
		last_client = prev_client;
	if (flag != 1){// try to delete unexist client
		printf("BUG-try to delete unexist client\n");
		Terminate(ERR);
	}
	/* client free */
	if ( delete_first != 1)
		prev_client->next = client_tcp->next;
	else
		client_list = client_tcp->next;

	clients_num --;
	client_tcp->live = 0 ;
}
void closing_socket(int socket){
	/*closing the socket */
	if ( close(socket) == -1)
		perror("Failed to close welcome socket \n");
}
int search_song(char * songName){
	/*the function check if the song is exist*/
	struct UDP_info * temp = station_list;
	int i, prefix=0;
	for (i=0 ;i< max_stations_num;i++){
		if (temp->from_user){// if the song is from the client cut the name
			prefix = PREFIX_LEN;
		}
		if (strcmp(songName,temp->song_name+prefix) == 0){
			return 1;
		}
		temp = temp->next;
		prefix = 0;
	}
	return 0;
}

