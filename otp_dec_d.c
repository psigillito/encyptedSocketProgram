#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

//Checks if any of the child processes have finished and waits on them if they did
//also returns the number of processes that have end to update child process count
int checkChildProcesses(int array[],int runningCount ){
	int returnFinished= 0;
	int i = 0;
	for(i=0;i<100;i++){

		if(array[i]!=0){
			int temp = waitpid(array[i],0,WNOHANG);
			if(temp != 0)
			{
				if(temp == -1){
					printf("Error cannt find function\n");
					fflush(stdout);
				}
	
				array[i]= 0;
				returnFinished++;
			}	
			
		}
	}
	
	return returnFinished;
}

//Takes two buffers and outputs a decrypted string 
char* decryptData(char textData[], char keyData[]){
	
	char translateValues [27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
	int dataLength = strlen(textData);
	char encryptedData[dataLength+1];	
	int i = 0;
	for(i=0; i<(dataLength);i++){
		if( (int)textData[i] == 10){
			break;
		}
			
		char targetChar = textData[i];
		
		int tempy = 0;
		int x;
		for(x=0; x<27;x++){
			if(targetChar == translateValues[x]){
				tempy = x;
				break;	
			}		
		}

		char keyChar = keyData[i];
		int keyTemp = 0;
		int y;
		for(y=0; y<27;y++){
			if(keyChar == translateValues[y]){
				keyTemp = y;
				break;	
			}		
		}

		int newValue = (tempy - keyTemp);
		if(newValue < 0){
			newValue = newValue + 27;
		}

		newValue = newValue%27;
		encryptedData[i] = translateValues[newValue];
	}

		
//	encryptedData
	char* returnValue = malloc(sizeof(char)*(dataLength));
	strcpy(returnValue, encryptedData);

	return returnValue;
};

//Confirm user entered correct number of args
void portEntered(int argc){
	if (argc != 2) {
		fprintf(stderr,"Please enter the port number\n");
		fflush(stderr);
		exit(1);
	}	
}

int main(int argc, char* argv[]){

	//pidArray used to store ids of background processes
	int pidArray [100] = {0};

	int childExitMethod = -5;
	pid_t spawnPid = -5;

	//ESTABLISH LISTENING PORT
	portEntered(argc);
	
	//Initialize struct for listening port per class lectures and Beej's Guide 
	struct sockaddr_in serverAddress, clientAddress;
	
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	int portNumber = atoi(argv[1]);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	
	//Set socket, bind, and listen for commmand connection per Beej's Guide
	int listenSocket = socket(AF_INET,SOCK_STREAM,0);
	int binder = bind(listenSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

	//error 
	if(binder == -1){
		fprintf(stderr, "cant connect to that port\n");
		fflush(stderr);
		exit(1);
	}
	
	if(listen(listenSocket, 10) == -1 ){
		fprintf(stderr, "error listening\n");
		fflush(stderr);
		exit(1);
	} 	
	

	// struct to contain host info of client per Beej's guide 
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	sin_size = sizeof their_addr;
	int childProcessCount = 0;

	//keep listening for connections
	while(1){
		//update child process count and if <5 start another in queue
		int childProcessFinished = checkChildProcesses(pidArray, childProcessCount);	
		childProcessCount = childProcessCount - childProcessFinished;	
		if( childProcessCount  < 5){
			
			//Establish connection by accepting client request 
			int new_connection = accept(listenSocket, (struct sockaddr*)&their_addr, &sin_size);

			//make child process
			spawnPid = fork();

			//add process to list of pids	
			if( spawnPid != 0){
				close(new_connection);
				int i = 0;
				for(i = 0; i < 100; i++){
					if(pidArray[i] == 0){
						pidArray[i] = spawnPid;
						childProcessCount++;
						i = 200;
					}
				}
			}

			//If we are in child process receive data
			if( spawnPid == 0){
				
				//Receive Handshake 
					char handshake[100];
					memset(handshake,'\0',100);
					int hCharsRead = 0;
			
					while(hCharsRead != 99){
						hCharsRead += recv(new_connection, hCharsRead + handshake, 99, 0);
					}

				//Parse what to send back. Send 'Y' if correct, 'N' if incorrect
					char rHandShake[100];
        				memset(rHandShake,'\0',100);

					if(strcmp(handshake, "B")==0){
						rHandShake[0]='Y';
					}else{
						rHandShake[0]='N';
					}
				//Send Back handshake saying connection approved or disapproved
        			int rBytesToSend = strlen(rHandShake)*sizeof(char);
        			int rSentAmount = 0;
        			while(rSentAmount < rBytesToSend){
               				rSentAmount += send(new_connection,rHandShake + rSentAmount,99,0);
       				}

				//Close connection if not correct client
				if(strcmp(handshake,"B")!=0){
					close(new_connection);
					exit(1);
				}
				
				//RECEIVE KEY
				//Style of sending data based on class lectures and comments on piazza.
				//specifically https://piazza.com/class/jc14n3c2sfz1wg?cid=537
					char buffer[200000];
        				memset(buffer, '\0', 200000);
        				int charsRead = 0;
        				
	       				while ( charsRead != 199999){
						charsRead += recv(new_connection, buffer + charsRead, 199999, 0);			
    					}
					//trim buffer
					buffer[strlen(buffer)-1] = '\0';	
		
				//RECEIVE TEXT
					char textBuffer[200000];
        				memset(textBuffer, '\0', 200000);
        				int textCharsRead = 0;
        				       			
					//Key receiving until all received 
					while ( textCharsRead != 199999){
	               				textCharsRead += recv(new_connection, textBuffer + textCharsRead, 199999, 0);
    					}
					
					//trim buffer
					textBuffer[strlen(textBuffer)-1]='\0';

				//ENCRYPT DATA
					char* decryptedResponse =  decryptData(textBuffer, buffer);

				//SEND ENCRYPTED DATA

					//Buffer for sending data 
					char decryptedOutputBuffer[200000];
        				memset(decryptedOutputBuffer, '\0',200000);
        		
					//Copy Data into Buffer for ease of sending 
					int x = 0;
					int max = strlen(decryptedResponse);
					for(x = 0; x < max; x++){
						decryptedOutputBuffer[x]= decryptedResponse[x];
					}
					
					//Number of bytes needed to be sent 
					int BytesToSend = strlen(decryptedOutputBuffer)*sizeof(char);
       					int sentAmount=0;

					//keep sending until all sent
	       				while(sentAmount < BytesToSend){
						sentAmount += send(new_connection,decryptedOutputBuffer+sentAmount,199999,0);
       			 		}
       	
				close(new_connection);
				exit(0);
			}
		}
	}
	return 0;
}