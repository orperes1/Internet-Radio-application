# Internet-Radio-application
Internet Radio Station that streams songs using multicast in a single AS. Client that will connect the radio station, play songs receive data from the radio station, and add new songs to the radio station. Server that support 100 users 

The server and client both communicate using the Protocol . If either of them misbehaves, that is, behaves is a way that is contrary to the protocol (wrong message types, wrong format, wrong timing), the connection will be terminated. Either side might trigger the disconnect. In either case, the server must remain online always, sending songs and be ready to receive more clients at any time.
The Server itself is a multithreaded machine, it is always online, it streams multiple songs and handles multiple clients at the same time. The server will need to handle multiple sources of input and output at the same time .
The Server streams several songs at the same time using multicast, with a single song for each multicast address.
The server handles several clients simultaneously; a client might disconnect or send a request at any time, and the server must handle each client properly, and know which clients are connected to it each moment.
The server also handles simple input from a user. The user may choose to close the server or print the server’s database.
Any connected client might also request to upload a new song to the station, when the song is uploaded successfully, the server begins playing it, and any client is able to listen to it.
The Client is the one listening to the radio. The Client plays the songs that are streamed from the server. The Client sends queries to the server (what song is played at each station), or request to upload a song to the server. The client also interacts with input from a human user (someone needs to ask it to change the station...).
