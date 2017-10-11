#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> //definitions of data types used in system calls
#include <sys/socket.h> //defines socket structures
#include <netinet/in.h> //defines constants and structures needed for internet domain access
#include <string.h>

/* void error(char* msg):
	Error function that handles errors resulting from system calls
		Credit: http://www.linuxhowtos.org/C_C++/socket.htm
*/
void error(char* msg){ 
	perror(msg);
	exit(1);
}

typedef struct location{
	char zip[10];
	char city[20];
	char state[2];
}location;

location* newLocation(char* zip){
	location* l = (location*) malloc(sizeof(location));
	strcpy(l->zip, zip);
	return l;
}

location* getZip(char* zip);

/* int main(int argc, char* argv[]):
	MAIN ROUTINE FOR SERVER:
	sockfd, newsockfd: file descriptor for a socket
	portno: the portnumber to listen on
	clientLen: length of the address of the client (needed for accept system call)
	n: return val for read and write. AKA the number of characters read or written
	socketfd: will be opened with AF_INET domain as it will need to facilitate communication
		between two hosts over the internet
		-It will be using the STREAM type and 0 for the protocol argument, which allows the
			system to choose (which will be TCP for stream)


	CREDIT: Designed with guidance from linuxhowtos: http://www.linuxhowtos.org/C_C++/socket.htm
*/
int main(int argc, char* argv[]){
	int sockfd, newsockfd, portno, clientLen, n; 
	char* zip;
	char city[31], state[10];
	char comma[3] = ", \0";
	char buffer[256]; //acts as the buffer for characters to be read into the server

	struct sockaddr_in serv_addr, cli_addr; //this structure contains an internet address corresponding to server and client respectively
	
	if(argc < 2){ //checks for port number
		fprintf(stderr, "ERROR, no port number entered\n");
		exit(1);
	}	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) 
		error("ERROR opening socket");
	bzero((char*) &serv_addr, sizeof(serv_addr)); //sets all values in a buffer to zero

	portno = atoi(argv[1]); //pull port number from command line
	if(portno < 2000 || portno > 65535){
		fprintf(stderr, "ERROR, port out of range (2000-65535)\n");
		exit(1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	
		//bind to socket
		if(bind(sockfd, (struct sockaddr *) &serv_addr, 
			sizeof(serv_addr)) < 0)
			error("ERROR on binding");
		listen(sockfd, 5);
		clientLen = sizeof(cli_addr);
		
	while(1){	
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientLen);
		if(newsockfd < 0)
			error("ERROR on accept");
		bzero(buffer, 256);

		//READ
		n = read(newsockfd, buffer, 255);
		if(n < 0) error("ERROR reading from socket");

		//FETCH
		zip = buffer;
		zip[strlen(zip)-1] = '\0';
		if(strlen(zip) != 5)
			break;
		location* result = getZip(zip);
		strcpy(city, result->city);
		strcat(city, comma);
		strcat(city, result->state);
		

		//WRITE
		n = write(newsockfd, city, sizeof(city));
		if(n < 0) error("ERROR writing to socket");
		
		
	}return 0;
	close(newsockfd);
	close(sockfd);
}

location* getZip(char* zip){
	location* l = newLocation(zip);
	FILE* file;
	file = fopen("cityzip.csv", "r");

	if(file == NULL){
		error("Could not open file\n");
	}

	char* token;
	char lineIn[100];
	int count = 1;
	fgets(lineIn, 100, file);

	char zipA[5];
	strcpy(zipA, zip);
	char newZip[6] = ",";
	strcat(newZip, zipA);

	while(count!=25774){
		fgets(lineIn, 100, file);
	

		if(strstr(lineIn, newZip) != NULL){ 
			token = strtok(lineIn, ",");
			strcpy(l->state, token);
			token = strtok(NULL, ",");
			strcpy(l->city, token);
			return l;
		}

		count++;
	}
	fclose(file);
	strcpy(l->city, "ZIP NOT FOUND");
	strcpy(l->state, " ");
	return l;

}

