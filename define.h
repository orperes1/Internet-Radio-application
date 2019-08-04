#define BUFFER_SIZE				1024
#define TIMEOUT1				300000
#define TIMEOUT2				2
#define TIMEOUT3				3
#define TRUE					1
#define SONG_NAME_SIZE			200
#define SONG_DIR_SIZE			230
#define REPLY_STRING_SIZE		1024
#define MEGA					1000000
#define ASK_SONG_SIZE			3
#define UP_SONG_SIZE			SONG_NAME_SIZE +6
#define TTL						10
#define OPT_RATE				62500

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
#define ANNOUNE					1
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


/*error types for server*/
#define HELLO_ERROR				0
#define DUP_HELLO_ERROR			1
#define INVALID					2
#define WRONG_SIZE				3
#define WRONG_ST				4
#define UP_TIMEOUT				5

/* 	MSG LEN */
#define WELCOME_LEN				9
#define NEWSTATIONS_LEN			3
#define PREFIX_LEN				20
#define PERMITSONG_LEN			2
