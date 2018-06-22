#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ioctl.h>

//Confirms correct number of files entered 
void argsEntered(int argc){
	if (argc != 4) {
		fprintf(stderr,"Please provide 3 arguments\n");
		fflush(stderr);
		exit(1);
	}	
}

//Makes Sure key file is longer than text file 
//Makes Sure no bad characters in fils 
//Exits and prints to stderr in error
void testFiles( char keyTestBuffer[] , char textTestBuffer []){

	int textTestLength = strlen(textTestBuffer);
	int keyTestLength = strlen(keyTestBuffer);

	if(textTestLength > keyTestLength){
		fprintf(stderr,"Text File Longer than Key File");
		fflush(stderr);
		exit(1);
	}

	int p = 0;
	for(p=0; p<textTestLength; p++){
		int testValue = (int)textTestBuffer[p];
		if( (testValue > 90 || testValue < 65) && testValue != 32 && testValue != 10){
			fprintf(stderr,"Bad Characters in Data File");
			fflush(stdout);
			exit(1);
		}

	}

	for(p=0; p<keyTestLength; p++){
		int testValue = (int)keyTestBuffer[p];
		if( (testValue > 90 || testValue < 65) && testValue != 32 && testValue != 10){
			fprintf(stderr,"Bad Characters in Key File");
			fflush(stdout);
			exit(1);
		}
	}
}


int main(int argc, char* argv[]){
	
	//confirm arg count
	argsEntered(argc);
	
	//confirm inputfiles before connecting to server
	FILE* keyTest;
	keyTest = fopen(argv[2], "r");
	
	FILE* textTest;
	textTest = fopen(argv[1], "r");
	
	char keyTestBuffer[200000];
	memset(keyTestBuffer, '\0',200000);
	fread(keyTestBuffer, sizeof(char),199999, keyTest);

	char textTestBuffer[200000];
	memset(textTestBuffer, '\0',200000);
	fread(textTestBuffer, sizeof(char),199999, textTest);

	testFiles( keyTestBuffer, textTestBuffer);


	//Setup Structs per class lectures and Beej's Guide	
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[3]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverHostInfo = gethostbyname("localhost");
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0){
		 error("CLIENT: ERROR opening socket");
	}

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		error("CLIENT: ERROR connecting");
	}	

	//SEND HANDSHAKE of 'A' If 'Y' returned good, if 'N' returned bad
	char handshake[100];
	memset(handshake, '\0',100);
	handshake[0]='A';
	int hBytesToSend = strlen(handshake)*sizeof(char);
	int hSentAmount = 0;
	
	while(hSentAmount < hBytesToSend){
		hSentAmount += send(socketFD,handshake + hSentAmount,99,0);
	}

	//Receive Handshake Message back and store into buffer
	char rHandShake[100];
        memset(rHandShake,'\0',100);
        int hCharsRead = 0;

        while(hCharsRead != 99){

        	hCharsRead += recv(socketFD, hCharsRead +rHandShake, 99, 0);
        }

	//Parse HandShakeResponse. If Response not 'Y' then incorrect and error out
	if(strcmp(rHandShake, "Y") !=0){
		fprintf(stderr,"Error, tried to connect to port%d\n",portNumber);
		fflush(stderr);
		close(socketFD);
		exit(2);
	}
		
	//SENDING KEY  
	//Style of sending data based on class lectures and comments on piazza. 
	//specifically https://piazza.com/class/jc14n3c2sfz1wg?cid=537
		//Open key file and read into buffer
		FILE* kp;
		kp = fopen(argv[2], "r");
		char keyOutputBuffer[200000];
		memset(keyOutputBuffer, '\0',200000);
		fread(keyOutputBuffer, sizeof(char),199999, kp);

		//Number of bytes needed to send
		int BytesToSend = strlen(keyOutputBuffer)*sizeof(char);

		int sentAmount=0;
		while(sentAmount < BytesToSend){
			sentAmount += send(socketFD,keyOutputBuffer+sentAmount,199999,0);
		}	
	
		//Very Large buffer so do ioctl to send all beore continuing, otherwise errors occur
		int checkSend = -5;  
		do
		{
  			ioctl(socketFD, TIOCOUTQ, &checkSend); 
  		}
  		while (checkSend > 0);  
  		if (checkSend < 0){  
  	  		error("ioctl error");
		}

	//SEND TEXT DATA
		
		//Read Text Data into buffer
		FILE* fp;
		fp = fopen(argv[1], "r");
		char textOutputBuffer[200000];
		memset(textOutputBuffer, '\0',200000);
		fread(textOutputBuffer, sizeof(char),199999, fp);

		//Number of bytes needed to send
		int TextBytesToSend = strlen(textOutputBuffer)*sizeof(char);
		int textSentAmount=0;

		while(textSentAmount < TextBytesToSend){
			textSentAmount += send(socketFD, textOutputBuffer+textSentAmount,199999,0);
		}

		//Vert Large buffer so do ioctl otherwise errors occur
		checkSend = -5;
		do
		{
  			ioctl(socketFD, TIOCOUTQ, &checkSend);
  		}
  		while (checkSend > 0);
  		if (checkSend < 0){
  	  		error("ioctl error");
		}

	//RECEIVE ENCRYPTED MESSAGE
        	
		//create buffer to receive message
		char encryptedTextBuffer[200000];
        	memset(encryptedTextBuffer, '\0', 200000);
        	int textCharsRead = 0; 
        	int textEndCharacter = 0;

		//keep getting message until buffer full
        	while ( textCharsRead != 199999)
        	{
        		textCharsRead += recv(socketFD, encryptedTextBuffer + textCharsRead, 199999, 0);
        	}
	
	//STDOUT MESSAGE
		printf("%s\n",encryptedTextBuffer);

	return 0;
}
