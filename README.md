# Tris
## Description
The project consists in developing a client/server TRIS GAME application. 
Both the server and the client must be mono-process and exploit the I/O multiplexing to handle multiple inputs and outputs simultaneously. 


During a game each player has one of the two symbols of the game: 'X' or 'O'.
The server will be responsible for storing the users connected and their listening ports.
The exchange of information between client and server will be through TCP sockets. 
This information will be used to allow P2P communication between clients through an UDP socket.

The game map is the following:
![Map](map.png)
		 
## Client
The client must be started with the following syntax
```
./tris_client <server address> <server port>
```
The commands available to the user should be:
```
!help - displays a list of available commands
!who - displays a list of connected users
!connect <username> - start a match with the user <username>
!disconnect - disconnects the client from the current game
!show_map - displays the game's map
!hit <num_cell> - tick checkbox <num_cell> (valid only when it is your turn)
!quit - disconnects the client from the server
```
While connecting the client has to enter his username and listen port for UDP commands for the game.

## Server
The program tris_server handles requests from clients. 
The server accepts new connections, registers new users and handles the requests of various 
clients to open new matches.

The command syntax is as follows:
```
./tris_server <address> <port>
```