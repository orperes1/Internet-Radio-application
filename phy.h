#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "commends.h"


int live;
int open_tcp_sock (char * server_name, uint16_t server_port);
void send_hello (int socket);
void * control_thread (void *pvoid);
void Terminate (int error_type);
void handle_massage();
void closing_socket(int socket);
void welcome_to_establish(Welcome welcome);
void ask_song_to_establish(Announce announce);
void new_station(NewStations newstation);
void up_song_to_sending ();
void * listener_thread(void *pvoid);
void ask_song_req(uint16_t num);
void up_song (char *song_name);
void change_station(uint16_t station_num);
