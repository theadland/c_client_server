#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include "utils.h"

#define debug_print(message, ...) do { if (DEBUG) fprintf(stderr, message, __VA_ARGS__); } while (0)

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

void handle_client(int client_socket, struct sockaddr_in clientAddress)
{
	debug_print("SERVER: Connected to client running at host %d port %d\n", 
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));

	
	// receive handshake
	char* handshake = calloc(256, sizeof(char));
	handshake = readMsg(handshake, client_socket);
	debug_print("SERVER: I received this HANDSHAKE from the client: \"%s\"\n", handshake);

	// if the wrong client connect, send it a message to close connection
	char* close_message = "close";
	char* good_message = "good";
	if (strcmp(handshake, "client")) {
		sendMsg(close_message, client_socket);
		fprintf(stderr, "Something other than enc_client tried to connect\n");
		close(client_socket);
		exit(0);
	} else {
		sendMsg(good_message, client_socket);
	}
	free(handshake);
	debug_print("SERVER: I sent this HANDSHAKE from the client: \"%s\"\n", good_message);
	
	// Loop to respond client loop; timing test
	char* msg = calloc(256, sizeof(char));
	while(strcmp(msg, "done")) {
		msg = readMsg(msg, client_socket);
		debug_print("SERVER: I received this message from the client: \"%s\"\n", msg);
		sendMsg("good", client_socket);
		debug_print("SERVER: I sent this message from the client: \"%s\"\n", "good");
	}
	free(msg);
	
	debug_print("SERVER: Closing socket %i and exiting client handler process\n", client_socket);
	// close connection
	close(client_socket);
}


int main(int argc, char *argv[]){
  int connectionSocket;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  
	/*Setup Networking Stuff*/
	/*********************************************************/
	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0) {
		error("ERROR opening socket");
	}

	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// Associate the socket to the port
	if (bind(listenSocket, 
			(struct sockaddr *)&serverAddress, 
			sizeof(serverAddress)) < 0){
		error("ERROR on binding");
	}

	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5); 
	debug_print("SERVER: listening socket setup on: %i\n", listenSocket);
	/*********************************************************/
	
  
  while(1) {
	// Accept a connection, blocking if one is not available until one connects
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
				
    if (connectionSocket < 0) {
      error("ERROR on accept");
    }
	debug_print("SERVER: accepted connection on listening socket on port %i\n", atoi(argv[1]));
	
	/* fork new process and execute command */
	pid_t childList[256];
	memset(childList, 0, 256);
	pid_t spawnpid = fork();
	switch (spawnpid) {
		case -1:
			perror("Failed to fork process");
			break;

		case 0:
			// Execute child process
			debug_print("SERVER: handle_client child process created on port %i\n", atoi(argv[1]));
			handle_client(connectionSocket, serverAddress);
			 
			// exit process
			exit(0);

		default:
			// managing process
			// Close the server's copy of connection socket for this client
			close(connectionSocket);
			
			debug_print("SERVER: new process id: %d\n", spawnpid);
			
			waitpid(-1, 0, WNOHANG);
	}

     
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
