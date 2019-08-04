#define BUFFER_SIZE				1024
#define TIMEOUT1				300000
#define TIMEOUT2				10
#define TIMEOUT3				3
#define TRUE					1
#define SONG_NAME_SIZE			200
#define REPLY_STRING_SIZE		1024
#define MEGA					1000000

/*client state*/
#define OFF 					0
#define WAIT_FOR_WELCOME		1
#define ESTABLISH				2
#define ASK_SONG				3
#define UP_SONG					4
#define SENDING_SONG			5

/*server state*/
#define S_OFF 					0
#define S_ESTABLISH				1
#define S_UP_SONG				2

/*msg types- from server*/
#define WELCOME 				0
#define ANNOUNCE				1
#define PERMITSONG				2
#define INVALIDCOMMAND 			3
#define NEWSTATIONS				4

/*msg types- from client*/
#define HELLO 					0
#define ASKSONG					1
#define UPSONG					2

/* error types */
#define ERR						0
#define ERR_UNEXPECTED_MASSGE	1
#define ERR_TIMEOUT				2
#define LEAVE					3

/* msg lens */
#define ASK_SONG_LEN			3

