/*
Translating Micro Server
Author: Alliana Dela Pena
Date: Oct 22, 2021
References: 
Tutorial Week 3 Content https://d2l.ucalgary.ca/d2l/le/content/401094/Home
*/

#include <iostream> 
#include <cstdlib> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h> 

#define MAX_MESSAGE_LENGTH 400

void translateWord(char french_word[], char eng_word[]);

int main(int argc, char const *argv[]){
    addrinfo server_info, *micro_info;

    char clientMessage[MAX_MESSAGE_LENGTH];
    char serverMessage[MAX_MESSAGE_LENGTH];
    char french_word[MAX_MESSAGE_LENGTH];

    //Initializing socket
    memset(&server_info, 0, sizeof(addrinfo));
    server_info.ai_family = AF_INET;
    server_info.ai_socktype = SOCK_DGRAM;
    server_info.ai_flags = AI_PASSIVE; // for wildcard IP address
    server_info.ai_protocol = 0; // any protocol

    getaddrinfo(NULL, "12000", &server_info, &micro_info);

    int status;

    //Create a socket
    int microSocket = socket(micro_info->ai_family, micro_info->ai_socktype, micro_info->ai_protocol);
    if(microSocket == -1){
        printf("socket() failed in micro server.");
        exit(1);
    }

    printf("Hello! I am the translate micro server!\n\n");
    printf("Accepted english words:\n- hello\n- dizzy\n- flower\n- potato\n- turtle\n\n");


    //Bind to the socket
    if ((status = bind(microSocket, micro_info->ai_addr, micro_info->ai_addrlen)) == -1){
        printf("bind() faild in micro server.");
        exit(1);
    }

    int done = 0;
    while(!done) {
        //Receive from indirect server
        memset(clientMessage, 0, sizeof(clientMessage));
        size_t recv_size, send_size; 
        if((recv_size = recvfrom(microSocket, clientMessage, sizeof(clientMessage), 0, micro_info->ai_addr, &micro_info->ai_addrlen)) < 0){
            printf("recvfrom() failed in micro server");
        }

        clientMessage[strlen(clientMessage)] = '\0';
        printf("Recieved from indirection server:\n<%s>\n\n", clientMessage);

        //If client ends session get out of loop and close socket
        if(strcmp(clientMessage, "0") == 0){
            done = 1;
        }
        else{
            //translating english word to french
            translateWord(french_word, clientMessage);
            sprintf(serverMessage, "French translation: <%s>\n\n------------Choose a service.-----------\n", french_word);
            serverMessage[strlen(serverMessage)] = '\0';

            printf("Sending to indirection server:\n<%s>\n\n", serverMessage);

            //Sending message to indirection server
            if ((send_size = sendto(microSocket, serverMessage, strlen(serverMessage), 0, micro_info->ai_addr, micro_info->ai_addrlen)) == -1){
                perror("sendto() failed");
                exit(1);
            } 
        }
    } 
    close(microSocket);
    fprintf(stderr, "bye!");
    exit(0);
}

void translateWord(char french_word[], char eng_word[]){
    if(strcmp(eng_word, "hello") == 0){
        strcpy(french_word, "bonjour");
    }
    else if(strcmp(eng_word, "dizzy") == 0){
        strcpy(french_word, "vertigineuse");
    }
    else if(strcmp(eng_word, "flower") == 0){
        strcpy(french_word, "fleur");
    }
    else if(strcmp(eng_word, "potato") == 0){
        strcpy(french_word, "patate");
    }
    else if(strcmp(eng_word, "turtle") == 0){
        strcpy(french_word, "tortue");
    }
    else{
        strcpy(french_word, "Sorry can't translate that word. Gotta study more french:)");
    }
}