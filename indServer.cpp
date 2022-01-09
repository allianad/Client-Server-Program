/*
Indirection Server
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
#include <inttypes.h>
#include <ctype.h>
#include <cstdlib> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream> 
#include <time.h>

using namespace std;


#define MAX_MESSAGE_LENGTH 400

//Global variables
int clientSocket, ind_servPort;
char micro_servPort[30];
char ind_servIP[100];
char clientMessage[MAX_MESSAGE_LENGTH];
char serverMessage[MAX_MESSAGE_LENGTH];

// This is a signal handler to do graceful exit if needed 
void catcher( int sig ){
	close(clientSocket);
	exit(0);
}
  
void toMicroServer();
void closeMS();

int main(int a, char *args[]){
  int i = 0;

	//Getting server IP
    while(args[1][i] != '-'){
        if(!isdigit(args[1][i]) && args[1][i] != '.'){
            printf("IP is not in the correct format");
            exit(1);
        }
        ind_servIP[i] = args[1][i];
        i++;
    }
	i++;
	ind_servIP[i] = '\0';

    //Getting server port number
    while(args[1][i] != '\0'){
        if(!isdigit(args[1][i])){
            printf("Port should be a digit.");
            exit(1);
        }
        ind_servPort = (args[1][i] - 48) + (10 * ind_servPort);
        i++;
    }

  fprintf(stderr, "port = %d\n",ind_servPort);
	fprintf(stderr, "IP = %s\n",ind_servIP);

	//Initialize socket 
    struct sockaddr_in ind_server, client;
    static struct sigaction act;
    int clientSocket, c;

	//Set up a signal handler to catch any unusual termination conditions.
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    //Initialize server sockaddr structure
    memset(&ind_server, 0, sizeof(ind_server));
    ind_server.sin_family = AF_INET;
    ind_server.sin_port = htons(ind_servPort);
    ind_server.sin_addr.s_addr = inet_addr(ind_servIP);

    //Create client socket
    if( (clientSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1 ){
		fprintf(stderr, "testserver: socket() call failed!\n");
		exit(1);
    }

    //Bind socket
    if( bind(clientSocket, (struct sockaddr *)&ind_server, sizeof(struct sockaddr_in) ) == -1 ){
		fprintf(stderr, "testserver: bind() call failed!\n");
		exit(1);
      }

    //Listen for client connections 
    if( listen(clientSocket, 5) == -1 ){
		fprintf(stderr, "testserver: listen() call failed!\n");
		exit(1);
    }

    /* initialize message strings just to be safe (null-terminated) */
    bzero(clientMessage, MAX_MESSAGE_LENGTH);
    bzero(serverMessage, MAX_MESSAGE_LENGTH);

    fprintf(stderr, "Hi I am the indirection server!!\n");
    fprintf(stderr, "Indirection server listening on TCP port %d...\n\n", ind_servPort);


    c = sizeof(struct sockaddr_in);
    
    //Listening forever for client message
    while(1){

		//Accept connection from client
		if( (clientSocket = accept(clientSocket, (struct sockaddr *)&client, (socklen_t *) &c)) == -1 )
		{
			fprintf(stderr, "testserver: accept() call failed!\n");
			exit(1);
		}
		else{
			printf("accept() success\n");
		}

			int c_recv_size;
			int done = 0;

      strcpy(micro_servPort, "0");

			while(!done){
				//Recieve messages from client
				c_recv_size = recv(clientSocket, clientMessage, MAX_MESSAGE_LENGTH, 0);

        printf("Recieved from client:\n<%s>\n\n", clientMessage);

        //if voting micro server was chosen set port
        if(strcmp(clientMessage, "3") == 0 || strcmp(clientMessage, "4") == 0 || strcmp(clientMessage, "5") == 0){
            strcpy(micro_servPort, "12002");
            strcpy(serverMessage, "");
          }

        //If client already chose a service
        if(strcmp(micro_servPort, "0") != 0){
            //Send client message to appropriate micro server and get serverMessage
            toMicroServer();

            //Finished with micro server so reset micro server port
            //Don't restart for the first time if not done voting
            if(strncmp(serverMessage, "key: ", 5) != 0){
                strcpy(micro_servPort, "0");
            }
        }

        //If client didn't choose a service previously
				//Find out which micro server client wants to use and set it to the correct port
        else{
          if(strcmp(clientMessage, "0") == 0){
            done = 1;
            printf("bye!");
            closeMS();
          }
          else if(strcmp(clientMessage, "1") == 0){
            strcpy(micro_servPort, "12000");
            strcpy(serverMessage, "Accepted english words:\n- hello\n- dizzy\n- flower\n- potato\n- turtle\nEnter an english word: ");
          }
          else if(strcmp(clientMessage, "2") == 0){
            strcpy(micro_servPort, "12001");
            strcpy(serverMessage, "Enter input in this format $<amount of money> CAD <destination currency>:\n");
          }
          else{
          //If micro server was not already chosen
          strcpy(micro_servPort, "0");
          strcpy(serverMessage, "Please choose 1, 2, or 3");
				}
        }      

        printf("Sending to client:\n<%s>\n\n", serverMessage);

        //Send the server message to the client
				if( send(clientSocket, serverMessage, strlen(serverMessage), 0) < 0)
				{
					fprintf(stderr, "Send() failed\n");
				}

				//Clear message strings
				bzero(clientMessage, MAX_MESSAGE_LENGTH);
				bzero(serverMessage, MAX_MESSAGE_LENGTH);
			}//while not done

		close(clientSocket);
    fprintf(stderr, "bye!");
		exit(0);
	}
}

  void toMicroServer(){

    //Initializing socket
    struct addrinfo client_info, *server_info;
    memset(&client_info, 0, sizeof client_info);
    client_info.ai_family = AF_INET; // set to AF_INET6 to use IPv6
    client_info.ai_socktype = SOCK_DGRAM;
    if ((getaddrinfo("136.159.5.25", micro_servPort, &client_info, &server_info)) != 0){
        fprintf(stderr, "getaddrinfo() failed.\n");
        exit(1);
    }

    //Create a socket
    int microSocket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    size_t send_size, recv_size;

    printf("Sending to micro server:\n<%s>\n\n", serverMessage);

    if ((send_size = sendto(microSocket, clientMessage, strlen(clientMessage), 0, server_info->ai_addr, server_info->ai_addrlen)) == -1){
        perror("sendto() failed");
        exit(1);
    }

    //Set timer to handle no response from micro server after 3 seconds
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if(setsockopt(microSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0){
      fprintf(stderr, "setsockopt failed\n");
    }

    if((recv_size = recvfrom(microSocket, serverMessage, sizeof(serverMessage), 0, server_info->ai_addr, &server_info->ai_addrlen)) < 0){
        printf("recvfrom() failed in micro server");
    }

    printf("Received from micro server:\n<%s>\n\n", serverMessage);

    //if no response from micro server notify client
    if(strcmp(serverMessage, "") == 0 ){
        strcpy(serverMessage, "No response from server it may be offline. Try again later.\n");
        sprintf(serverMessage, "%s\n\n------------Choose a service.-----------\n", serverMessage);
        serverMessage[strlen(serverMessage)] = '\0';
    }
 
    close(microSocket);
    return;
  }

  void closeMS(){
    strcpy(micro_servPort, "12000");
    for(int i = 0; i < 3; i++){

      //Send to conversion and voting microservers
      if(i == 1){
        strcpy(micro_servPort, "12001");
      }
      else if( i == 2){
        strcpy(micro_servPort, "12002");
      }


      //Initializing socket
      struct addrinfo client_info, *server_info;
      memset(&client_info, 0, sizeof client_info);
      client_info.ai_family = AF_INET; // set to AF_INET6 to use IPv6
      client_info.ai_socktype = SOCK_DGRAM;
      if ((getaddrinfo("136.159.5.25", micro_servPort, &client_info, &server_info)) != 0){
          fprintf(stderr, "getaddrinfo() failed.\n");
          exit(1);
      }

      //Create a socket
      int microSocket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

      size_t send_size, recv_size;

      printf("Sending to micro server:\n<%s>\n\n", serverMessage);

      if ((send_size = sendto(microSocket, clientMessage, strlen(clientMessage), 0, server_info->ai_addr, server_info->ai_addrlen)) == -1){
          perror("sendto() failed");
          exit(1);
      }

      close(microSocket);
    }
  }