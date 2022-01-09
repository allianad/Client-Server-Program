/*
Client
Author: Alliana Dela Pena
Date: Oct 22, 2021
References: 
Tutorial Week 3 Content https://d2l.ucalgary.ca/d2l/le/content/401094/Home
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <inttypes.h>
#include <ctype.h>


#define MAX_MESSAGE_LENGTH 400
int serverPort;
char serverIP[100];


int main(int a, char *args[]) {
    int i = 0;

    //Getting server IP
    while(args[1][i] != '-'){
        if(!isdigit(args[1][i]) && args[1][i] != '.'){
            printf("IP is not in the correct format");
            exit(1);
        }
        serverIP[i] = args[1][i];
        i++;
    }
	i++;
	serverIP[i] = '\0';

    //Getting server port number
    while(args[1][i] != '\0'){
        if(!isdigit(args[1][i])){
            printf("Port should be a digit.");
            exit(1);
        }
        serverPort = (args[1][i] - 48) + (10 * serverPort);
        i++;
    }

	printf("Hello client!\n\n");

	struct sockaddr_in client;
	int socket_desc, len, bytes, done;
	char c;
	char clientMessage[MAX_MESSAGE_LENGTH];
	char serverMessage[MAX_MESSAGE_LENGTH];

	bzero(serverMessage, MAX_MESSAGE_LENGTH);
	bzero(clientMessage, MAX_MESSAGE_LENGTH);


	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0); 
	if (socket_desc == -1)
		{
			fprintf(stderr, "Could not create socket, socket() failed");
		}

	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(serverPort);
	client.sin_addr.s_addr = inet_addr(serverIP); 

	//Connect to indirection server using TCP
	if (connect(socket_desc , (struct sockaddr *)&client , sizeof(struct sockaddr_in)) == -1 ) {
			fprintf(stderr, "connect error, connect() failed");
			return 1;
	}

	//Prompt user to choose a service
	printf("To choose a service by entering the one digit number.\n");
	printf("0 End Session.\n1 Translate english word to french.\n2 Convert Canadian dollars to another currency.\n");
	printf("3 Show candidates.\n4 Vote.\n5 View Voting Results.\n");
	printf("\n------------Choose a service.-----------\n");

	done = 0;
	while( !done){

		//Getting user input
		len = 0;
		while( (c = getchar()) != '\n'){
			clientMessage[len] = c;
			len++;
		}

		//Make sure the clientMessage is null-terminated in C
		clientMessage[len] = '\0';

		//Encrypt client message if they are voting
		if(strncmp(serverMessage, "key: ", 5) == 0){
			int id = 0;
			int key = 0;

			//Change vote to integer
			for(int i = 0; clientMessage[i] != '\0'; i++){
				id = (clientMessage[i] - 48) + (10 * id);
			}

			//Change key to integer
			for(int i = 5; serverMessage[i] != '\0'; i++){
				key = (serverMessage[i] - 48) + (10 * key);
			}

			//Encrypt vote
			id = id * key;

			//Change back to string
			bzero(clientMessage, MAX_MESSAGE_LENGTH);
			clientMessage[strlen(clientMessage)] = '\0';
			sprintf(clientMessage, "%d", id);
		}

		//Send it to the indirection server via the socket
		if(send(socket_desc, clientMessage, MAX_MESSAGE_LENGTH, 0) < 0) {
			printf("send() failed");
			return 1;
		}

		//Recieving message from indirection server
		if((bytes = recv(socket_desc, serverMessage, MAX_MESSAGE_LENGTH, 0)) > 0){
			serverMessage[bytes] = '\0';

			//Make sure not to print encryption key
			if(strncmp(serverMessage, "key: ", 5) != 0){
				printf("%s", serverMessage);
			}
			else{
				printf("You many now enter the ID of the person you want to vote for:\n");
			}

		}
		else{
			close(socket_desc);
			fprintf(stderr, "bye!");
			exit(1);
		}

		//check if session done
		if( strncmp(clientMessage, "0", 3) == 0){
			done = 1;
		}
	}


	close(socket_desc);
	exit(0);
  
  }