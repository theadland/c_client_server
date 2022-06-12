#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define debug_print(message, ...) do { if (DEBUG) fprintf(stderr, message, __VA_ARGS__); } while (0)
	
// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

char* readMsg(char* message, int socketFD)
{
	int charsRead;					// characters read during one loop iteration
	int bufferSize = 1024;			// receiving buffer size
	char buffer[bufferSize];		// buffer to store chars each loop	
	size_t remaining;				// chars left to read, loop condition

	
	// get message length
	size_t msgLen;
	int readLen = recv(socketFD, &msgLen, sizeof(size_t), 0);
	debug_print("readMsg: msgLen is %zu\n", msgLen);
	
	remaining = msgLen;
	
	// reallocate appropriate size
	message = realloc(message, msgLen);
	if(message == NULL)
		perror("ERROR reallocating readMsg buffer");
	memset(message, '\0', msgLen);

	
	while(remaining) {
		
		memset(buffer, '\0', bufferSize);
		charsRead = recv(socketFD, buffer, remaining, 0); 
		
		// Problem reading from socket
		if (charsRead <= 0){
			if (charsRead == 0) {
				fprintf(stderr, "readMsg: problem while receiving message, connection closed\n");
				break;
			} else {
				perror("ERROR reading from socket");
			}
		} 
		
		// add chars in buffer to output message
		strcat(message, buffer);
		remaining -= charsRead;
		
		//debug_print("readMsg: readMsg buffer:\n %s\n", buffer);
		
	}
	return message;
}

// sendMsg
void sendMsg(char* message, int socketFD)
{
	int sent = 0;						// total characters sent
	int charsWritten = 0;				// characters written during one loop iteration 
	size_t msgLen = strlen(message);	// message length
	int remaining = msgLen;				// loop condition

	// first send the length of the message
	int writeLen = send(socketFD, &msgLen, sizeof(size_t), 0);
	debug_print("sendMsg: msgLen is %zu\n", msgLen);
	
	while(remaining) {
		
		charsWritten = send(socketFD, message+sent, remaining, 0);
		
		if (charsWritten < 0)
			perror("ERROR writting message to socket");
		
		// update loop condition with remaining chars to send
		sent += charsWritten;
		remaining = msgLen - sent;
	}

}

// check input file for illegal characters
void checkFile(char* filename, char* buffer)
{	
	
  // check inputs and copy file to buffer
  FILE* fp = fopen(filename, "r"); 
  if(fp == NULL)
	  error("CLIENT: ERROR checking input file");
  
  char c = fgetc(fp);
  int ind = 0;
  while(!feof(fp)) {
		if ((c > 64 && c < 91) || c == ' ') {
			buffer[ind] = c;
			c = fgetc(fp);
			ind++;
		} else if (c = '\n') {
			c = fgetc(fp);
			continue;
		} else {
			fprintf(stderr, "invalid character found in input file\n");
			exit(1);
		}
    }
	buffer[ind] = '@';
    fclose(fp);
}

// get file length
long getFileLen(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if(fp == NULL)
	  error("CLIENT: ERROR opening filename for length");
  
	if(fseek(fp, 0, SEEK_END) < 0)
		error("CLIENT: ERROR seeking input file length");
	
	long file_len = ftell(fp);
	fclose(fp);
	return file_len;
}