#include "phy.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <math.h>


int tcp_socket=0,state , num_stations,udp_socket=0;
pthread_t control, listener ;
uint16_t cur_station;
uint16_t max_station_num=1;
uint32_t ip_start;
char song[SONG_NAME_SIZE];
struct ip_mreq mreq;
int orrrrrrr =1;

int open_tcp_sock (char * server_name, uint16_t server_port){
	/* the function receive the server info and open a tcp socket to the server */
	int error;
	struct sockaddr_in reciver ;
	state = OFF;
	/*open a socket and set its properties*/
	tcp_socket= socket(AF_INET,SOCK_STREAM,0);
	if (tcp_socket < 0){
		perror("Failed to open socket\n");
		Terminate(ERR);
	}

	/* set the sender info */
	reciver.sin_family =  AF_INET;
	reciver.sin_port = htons(server_port);
	reciver.sin_addr.s_addr = inet_addr(server_name);

	/* Connect to server */
	error = connect(tcp_socket,(struct sockaddr *) &reciver,sizeof(reciver));
	if (error < 0) {
		perror("Failed to connect to server \n");
		Terminate(ERR);
	}
	return tcp_socket;
}

void send_hello (int socket){/* the function send Hello massage */
	char buffer [BUFFER_SIZE]={0};
	int error ;

	printf("Sending hello msg\n");

	error = send(socket,buffer,3,0);
	if (error < 0 ) {
		perror("Failed to send hello \n");
		exit (0);
	}
	state = WAIT_FOR_WELCOME;
}

void * control_thread (void *pvoid){
	fd_set fdset;
	int inputfd ;
	char  user_command[200] ={0},comm;
	struct timeval time;
	int snum;
	while (live){
		//define timeout for each state
		switch (state){
			case (SENDING_SONG):{
				time.tv_usec = 0;
				time.tv_sec = TIMEOUT2;
				break;
			}
			case (ESTABLISH) :{
				time.tv_sec = 99999;
				time.tv_usec = 0;
				printf("Choose from the following options: \n * Number in order to switch station \n *'s' in order to up song \n *'q' in order to quit \n ");
				break;
			}
			case (WAIT_FOR_WELCOME) :{
				time.tv_sec = 0;
				time.tv_usec = TIMEOUT1;
				break;
			}
		}
		//define set of file descriptors
		FD_ZERO(&fdset);
		FD_SET(fileno(stdin),&fdset);
		FD_SET(tcp_socket,&fdset);

		//for each interput - enter to specific function
		inputfd = select(tcp_socket+1,&fdset,NULL,NULL,&time);
		if (inputfd == -1){
			printf("Error while select function!\n");
			Terminate(ERR);
		}
		else if(inputfd == 0){
			Terminate(ERR_TIMEOUT);
		}
		else if(inputfd > 0){
			if (FD_ISSET(fileno(stdin),&fdset) ) {
				gets(user_command);
				fflush(stdin);
				comm =user_command[0];
				switch (comm){
				case ('S'):
				case ('s'):{
					printf("Enter the song name - up to 200 characters\n");
					scanf("%s",song);
					up_song(song);
					break;
				}
				case ('Q'):
				case ('q'):{
					Terminate(LEAVE);
					break;
				}
				default:{
					if ((comm >= 'a' && comm <= 'z') || (comm >= 'A' && comm <= 'Z') || (comm == ' ')){
						printf("Invalid command\n");
						continue;
						break;
					}
					snum= atoi(user_command);
					if ( (snum >= 0) && (snum < max_station_num) ){ //check if the station num is in the range
						printf("Send ask song\n");
						ask_song_req(snum);
						printf("Changing station number: %d\n",snum);
						change_station(snum);
					} else{
						printf("Max station number is : %d station not in range \n",max_station_num-1);
						continue;
						break;
					}
				}
				}
			}
			if (FD_ISSET(tcp_socket,&fdset)){
				handle_massage();
			}

	}}
	return (void*)1;
}

void Terminate (int error_type){
	//print error massages
	switch (error_type){
		case (LEAVE):{
			printf("You ask to leave!\n");
			break;
		}
		case(ERR):{
			break;
		}
		case(ERR_UNEXPECTED_MASSGE):{
			printf("The server send Unexpected massage!\n");
			break;
		}
		case(ERR_TIMEOUT):{
			//check in which state we was
			printf("Receiving TIMEOUT\n");
			printf("State:%d \n",state);
			if(state == WAIT_FOR_WELCOME)
				printf("Missing WELCOME massage , timeout!\n");
			if(state == ASK_SONG)
				printf("The server not respond to ask song request!\n");
			if(state == UP_SONG)
				printf("The server not respond with permit respond!\n");
			if(state == SENDING_SONG)
				printf("The server not respond to sending data!\n");
			break;
		}
		default: {
			printf("Unexpected error massage!\n");
			break;
		}
	}

	/*close sockets*/
	if(udp_socket > 0)
		closing_socket(udp_socket);
	if(tcp_socket > 0)
		closing_socket(tcp_socket);

	live = 0;
	sleep(1);
	//indicate to close succesfuly
	printf("Goodbye!\n");
}

void handle_massage(){
	char buffer[BUFFER_SIZE] ,msg_type;
	int error,i;
	/* get massage from the server*/
	error = recv(tcp_socket,buffer, BUFFER_SIZE,0);
	if (error < 0) {
		perror("Failed to receive data \n");
		Terminate(ERR);
	}else if (error == 0){
		printf("The server close the connection \n");
		Terminate(ERR);
	}
	msg_type = buffer[0];
	switch(msg_type){
		//fill the massage's struct
		case(WELCOME):{
			printf("WELCOME !!!!\n");
			if (state != WAIT_FOR_WELCOME)
				Terminate (ERR_UNEXPECTED_MASSGE);
			struct Welcome welcome;
			welcome.replyType = WELCOME;
			welcome.numStations =  (buffer[1] << 8) | buffer[2];
			welcome.multicastGroup = (buffer[3] << 24) + (buffer[4] << 16) + (buffer[5] << 8) + buffer[6] ;
			welcome.portNumber = (buffer[7] << 8) | buffer[8];
			for (i=0;i<9;i++){
				welcome.welcome_word[i]=buffer[i];
			}
			welcome_to_establish(welcome);
			break;
		}
		case(ANNOUNCE):{
			printf("ANNOUNE !!!!\n");
			if (state != ASK_SONG) Terminate (ERR_UNEXPECTED_MASSGE);
			struct Announce announce;
			announce.replyType = ANNOUNCE;
			announce.songNameSize = buffer[1];
			strcpy(announce.songName,(char *)(buffer + 2) );
			ask_song_to_establish(announce);
			break;
		}

		case(PERMITSONG):{
			printf("PERMITSONG !!!!\n");
			if (state != UP_SONG) Terminate (ERR_UNEXPECTED_MASSGE);
			struct PermitSong permitSong;
			permitSong.replyType = PERMITSONG;
			permitSong.permit = buffer[1];
			if (permitSong.permit ) up_song_to_sending();

			/* if the user don't has permission*/
			else {
				printf("Sorry you don't have permission to upload a song /n");
				state = ESTABLISH;
			}
			break;
		}

		case(INVALIDCOMMAND):{
			printf("INVALID COMMAND !!!!\n");
			struct InvalidCommand invalid;
			invalid.replyType = INVALIDCOMMAND ;
			invalid.replyStringSize = buffer[1];

			strcpy(invalid.replyString , (char * ) (buffer +2 ));
			printf("%s\n",invalid.replyString);
			Terminate(ERR);
			break;
		}

		case(NEWSTATIONS):{
			printf("NEWSTATIONS !!!!\n");
			struct NewStations newstation;
			newstation.replyType = NEWSTATIONS ;
			newstation.newStationNumber = (buffer[1] << 8) + buffer[2] ;
			new_station(newstation);
			break;
		}
		default : {
			state = OFF;
			Terminate (ERR_UNEXPECTED_MASSGE);
			break;
		}
	}
	memset(buffer,0,BUFFER_SIZE);
	//control_thread(&control);
}

void ask_song_to_establish(Announce announce){
	printf("The song name is: %s\n",announce.songName);
	state = ESTABLISH;
}



void welcome_to_establish(struct Welcome welcome){
	char ip_addr[100];

	/* open UDP socket */
	max_station_num = welcome.numStations;
	int error;
	struct sockaddr_in reciver ;
	printf("Number of station is :%d\n",welcome.numStations);
	printf("Multicast port :%d\n",welcome.portNumber);
	/* open a socket and set its properties */
	udp_socket= socket(AF_INET,SOCK_DGRAM,0);
	if (udp_socket < 0){
		perror("Failed to open socket\n");
		Terminate(ERR);
	}

	/* set the receiver info */
	memset(&reciver,0,sizeof(reciver));
	reciver.sin_family =  AF_INET;
	reciver.sin_port = htons(welcome.portNumber);
	reciver.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(reciver.sin_zero, '0', sizeof reciver.sin_zero);

	/*bind*/
	error = bind(udp_socket,(struct sockaddr *) &reciver, sizeof(reciver));
	if (error < 0 ){
		perror("Failed to bind2\n");
		Terminate(ERR);
	}
	ip_start=inet_addr(ip_addr);

	sprintf(ip_addr,"%d.%d.%d.%d\n",welcome.welcome_word[6],welcome.welcome_word[5],welcome.welcome_word[4],welcome.welcome_word[3]);
	printf("Multicast addrs :%s\n",ip_addr);
	/* set parameters to MCAST group */
	mreq.imr_multiaddr.s_addr = inet_addr(ip_addr);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	/*connect to MCAST group */
	error = setsockopt(udp_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
	if (error < 0 ){
		perror("Failed to set socket opt\n");
		Terminate(ERR);
	}

	state = ESTABLISH;
	pthread_create(&listener,NULL,&listener_thread,NULL);
	control_thread(&control);
}

void closing_socket(int socket){
	/*closing the socket */
	printf("Closing socket \n");
	if ( close(socket) == -1) {
		perror("Failed to close welcome socket \n");
	} else
		printf ("Success closing socket! \n");
}

void new_station(NewStations newstation){
	printf("New station added - the station number is:%d\n",newstation.newStationNumber);
	max_station_num++;

	if(state == SENDING_SONG)
		state = ESTABLISH;
}


void up_song_to_sending (){
//send song in TCP connection
	FILE *fp;
	uint32_t supB , error, filesize;
	char buffer[BUFFER_SIZE];
	state = SENDING_SONG;

	fp = fopen(song, "r");
	if (fp == NULL){
		perror("Failed to open song \n");
		state = ESTABLISH;
		return;
	}

	/*sending file */
	printf("Start sending song to server \n");
	fseek(fp, 0L, SEEK_END);
	supB = ftell(fp);
	rewind(fp);
	filesize =supB;
	while (supB > BUFFER_SIZE ) {
		if (fscanf(fp,"%1024c", buffer) <0){
			perror("Failed to read file \n");
			Terminate(ERR);
		}
		printf("Sending  %.0f%% \r",(double)(filesize-supB)/filesize*100);
		fflush(stdout);
		error = send(tcp_socket,buffer,BUFFER_SIZE,0);
		if (error < 0 ) {
			perror("Failed to send file \n");
			Terminate(ERR);
		}
		supB = supB - error;
		sleep(0.7);
	}
	if (supB >0){
		if (fscanf(fp,"%1024c", buffer)<0){
			perror("Failed to read file \n");
			Terminate(ERR);
		}

		error = send(tcp_socket,buffer,supB,0);
		if (error < 0 ) {
			perror("Failed to send file \n");
			Terminate(ERR);
		}

	}

	/*closing the file */
	if (fclose(fp) == EOF){
		perror("Failed to close file \n");
		Terminate(ERR);
	}

}


void * listener_thread(void *pvoid){
	char buffer[BUFFER_SIZE];
	int error ;
	FILE *song;
	socklen_t len;
	struct sockaddr_in reciver ;

	state = ESTABLISH;
	/* start receiving data */
	song = popen("play -t mp3 -> /dev/null 2>&1", "w");
	while (live) {
		len = sizeof(reciver);
		error = recvfrom(udp_socket,buffer, BUFFER_SIZE,0,(struct sockaddr *) &reciver, &len);
		if (error < 0) {
			perror("Failed to receive data \n");
			Terminate(ERR);
		}
		if (error == BUFFER_SIZE){
			//play song
			fwrite(buffer,sizeof(char),BUFFER_SIZE,song);
		}
	}
	return (void*)1;
}

void change_station(uint16_t station_num){
	int error;
	int offset;

	offset = station_num - cur_station ;

	error = setsockopt(udp_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)); //drop membership
	if (error < 0) {
		perror("Failed to leave multicast group \n");;
		Terminate(ERR);
	}

	/* set parameters to MCAST group */
	mreq.imr_multiaddr.s_addr = htonl(ntohl(mreq.imr_multiaddr.s_addr) + offset);
	error = setsockopt(udp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)); //join new mgroup
	if (error < 0) {
		perror("Failed to join multicast group \n");
		Terminate(ERR);
	}

	cur_station = station_num;
}

void up_song (char *song_name){
	uint8_t buffer[sizeof(song)];
	struct UpSong song;
	int error,i;
	FILE *fd_song;

	gets(buffer);

	//Insert song information
	song.commandType =2;
	song.songNameSize= strlen(song_name);

	for (i=0;i<strlen(song_name);i++){
		song.songName[i] = song_name[i];
		buffer[i+6] = song_name[i];
	}

	fd_song = fopen(song_name,"r");
	if (fd_song == 0){
		printf("File doesn't exist\n");
		return ;
	}
	fseek(fd_song, 0L, SEEK_END);
	song.songSize = ftell(fd_song);
	rewind(fd_song);

	if( (song.songSize<2000) || (song.songSize>10*MEGA) ){
		printf("The size of the song is not in limits!\n");
		return;
	}

	buffer[0] = song.commandType;
	*(uint32_t*)&buffer[1] = htonl(song.songSize);
	buffer[5] = song.songNameSize;

	printf("sending request to upload song\n");

	error = send(tcp_socket,&buffer,6+song.songNameSize,0);
	if (error < 0 ) {
		perror("Failed to send up_song request \n");
		Terminate(ERR);
	}
	if (fclose(fd_song) == EOF){
		perror("Failed to close file \n");
		Terminate(ERR);
	}
	state = UP_SONG;
}

void ask_song_req(uint16_t num){ // the function sent the ask song msg
	int error ;
	uint8_t buffer[3];

	/*insert details */
	buffer[0]=1;
	*(uint16_t*)&buffer[1] = htons(num);

	error = send(tcp_socket,&buffer,ASK_SONG_LEN,0);
	if (error < 0 ) {
		perror("Failed to send file \n");
		exit (0);
	}
	state = ASK_SONG;
}

