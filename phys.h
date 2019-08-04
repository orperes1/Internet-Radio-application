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

int state, clients_num,tcp_port;
uint16_t max_stations_num,udp_port;
struct UDP_info * station_list ,*last_station;
struct TCP_info * client_list , *last_client ;
char mcast_ip[36];

int open_tcp_sock(in_addr_t server_name, uint16_t server_port);
void * play_th(void * station_num );
void print_data();
void Terminate(int error_type);
void new_client_add(TCP_info * new_client,struct sockaddr_in server);
void * client_handler(void *new_client);
void send_welcome_msg(TCP_info *sock_data);
void station_to_name(uint16_t station_num,TCP_info *sock_data);
int send_permit(int respond ,UpSong client_UpSong,TCP_info *sock_data);
void download_song(UpSong client_UpSong,TCP_info *sock_data);
void open_udp_socket(UDP_info *udp_station, char *song_name, char *ip_ad,int udp_port,int num) ;
void closing_socket(int socket);
void delete_client(TCP_info* client_tcp, int error);
void send_new_station(char * song_name);
void send_invalid_msg(int error_type, TCP_info * sock);
int search_song(char * songName);
