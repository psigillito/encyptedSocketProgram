#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>


void userEnteredLength(int argc){
	if(argc != 2){
		fprintf(stderr, "Please enter a length\n");
		fflush(stderr);
		exit(1);
	}
}


int main(int argc, char* argv[]) {

	char translateValues [27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};

	userEnteredLength(argc);
	int enteredCount = atoi(argv[1]);

	char outPutValues[enteredCount+1];
	int outputIndex = 0;

	srand(time(NULL));

	int i=0;
	for(i=0; i<enteredCount; i++){
		
		int randValue = rand()%(27);
		
		outPutValues[outputIndex] = translateValues[randValue];
		outputIndex++;
	}
	outPutValues[enteredCount]= '\n';
	printf(outPutValues);
	fflush(stdout);
	return 0;
}
