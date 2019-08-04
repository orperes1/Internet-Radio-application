/* structs for client*/
#include <stdint.h>
#include "define.h"

typedef struct Hello {
	uint8_t commandType; // 0
	uint16_t reserved;
} Hello;

typedef struct AskSong {
	uint8_t commandType;
	uint16_t stationNumber;
	char ask_song_word[3];
} AskSong;

typedef struct UpSong {
	uint8_t commandType; //2
	uint32_t songSize; //in bytes
	uint8_t songNameSize;
	char songName[SONG_NAME_SIZE];
} UpSong;

/* structs for server*/

typedef struct Welcome {
	uint8_t replyType; //0
	uint16_t numStations;
	uint32_t multicastGroup;
	uint16_t portNumber;
} Welcome;

typedef struct Announce {
	uint8_t replyType; //1
	uint8_t songNameSize;
	char songName[SONG_NAME_SIZE];
} Announce;

typedef struct PermitSong {
	uint8_t replyType; //2
	uint8_t permit;
} PermitSong;

typedef struct InvalidCommand {
	uint8_t replyType; //3
	uint8_t replyStringSize;
	char replyString[REPLY_STRING_SIZE];
} InvalidCommand;

typedef struct NewStations {
	uint8_t replyType; //4
	uint16_t newStationNumber;
} NewStations;

typedef struct UDP_info {
	char song_name[SONG_DIR_SIZE];
	FILE *fp;
	uint16_t num_station;
	struct sockaddr_in ip_addr;
	pthread_t station_th;
	int socket;
	struct ip_mreq mreq;
	uint8_t live;
	uint8_t from_user;
	struct UDP_info *next;
} UDP_info;

typedef struct TCP_info {
	int socket;
	char ip_addr[36];
	pthread_t client_th;
	struct sockaddr_in client_sock;
	uint8_t live;
	struct TCP_info *next;
} TCP_info;

