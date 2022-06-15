#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <time.h>
#include "utils.h"

#define debug_print(message, ...) do { if (DEBUG) fprintf(stderr, message, __VA_ARGS__); } while (0)

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;

  
	// Check usage & args
	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	} 
  
  
	/*Setup Connection to server*/
	/*********************************************************/
	// Create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0){
		error("CLIENT: ERROR opening socket");
	}

	// Set up the server address struct
	setupAddressStruct(&serverAddress, atoi(argv[1]), "localhost");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		error("CLIENT: ERROR connecting");
	}
	/*********************************************************/
  

	// Send handshake to server
	char* handshake = "client";
	sendMsg(handshake, socketFD);
	debug_print("CLIENT: I sent this HANDSHAKE to the server: \"%s\"\n", handshake);
	
	// Receive handshake
	char* handback = calloc(256, sizeof(char));
	handback = readMsg(handback, socketFD);
	debug_print("CLIENT: I received this HANDSHAKE from the server: \"%s\"\n", handback);
	
	// interpret handshake
	if (!strcmp(handback, "close")) {
		fprintf(stderr, "CLIENT: ERROR tried to connect to wrong server on port %i\n", portNumber);
		close(socketFD);
		exit(2);
	} else if (strcmp(handback, "good")) {
		fprintf(stderr, "CLIENT: ERROR bad handshake on port %i\n", portNumber);
		close(socketFD);
		exit(2);
	} else {
		// good handshake
	}
	free(handback);
	debug_print("CLIENT: Good handshake on port %i\n", portNumber);
	
	
	// send message loop
	int count = 256;
	char* sMsg = "test";
	char* rMsg = malloc(256);
	clock_t start = clock();
	while(count) {
		sendMsg(sMsg, socketFD);
		debug_print("CLIENT: I sent this message to the server: \"%s\"\n", sMsg);
		rMsg = readMsg(rMsg, socketFD);
		debug_print("CLIENT: I received this message from the server: \"%s\"\n", rMsg);
		count--;
	}
	sendMsg("done", socketFD);
	clock_t diff = clock() - start;
	
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	int avgt = msec / 256;
	printf("Total time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
	printf("Average time per message in milliseconds %d\n", avgt/1000);


  // Close the socket
  close(socketFD); 
  return 0;
}