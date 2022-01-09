/*
Currency Conversion Micro Server
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

void convertCurr(char clientMessage[], char serverMessage[]);

//Currency exchange values based on 1 CAD
double USD = 0.81;
double EUR = 0.70;
double GBP = 0.59;
double BTC = 0.000016;

char clientMessage[MAX_MESSAGE_LENGTH];
char serverMessage[MAX_MESSAGE_LENGTH];

int main(int argc, char const *argv[]){
    addrinfo server_info, *micro_info;

    //Initializing socket
    memset(&server_info, 0, sizeof(addrinfo));
    server_info.ai_family = AF_INET;
    server_info.ai_socktype = SOCK_DGRAM;
    server_info.ai_flags = AI_PASSIVE; // for wildcard IP address
    server_info.ai_protocol = 0; // any protocol

    getaddrinfo(NULL, "12001", &server_info, &micro_info);

    int status;

    //Create a socket
    int microSocket = socket(micro_info->ai_family, micro_info->ai_socktype, micro_info->ai_protocol);
    if(microSocket == -1){
        printf("socket() failed in micro server.");
        exit(1);
    }

    printf("Hello! I am the currency converter micro server!\n\n");

    //Bind to the socket
    if ((status = bind(microSocket, micro_info->ai_addr, micro_info->ai_addrlen)) == -1){
        printf("bind() faild in micro server.");
        exit(1);
    }

    printf("---Currency Exchange Table---\n");
    printf("   CAN to USD = 0.810000\n");
    printf("   CAN to EUR = 0.700000\n");
    printf("   CAN to GBP = 0.590000\n");
    printf("   CAN to BTC = 0.000016\n\n");

    int done = 0;
    while(!done) {
        //Receive from indirect server
        memset(clientMessage, 0, sizeof(clientMessage));
        size_t recv_size, send_size; 

        if((recv_size = recvfrom(microSocket, clientMessage, sizeof(clientMessage), 0, micro_info->ai_addr, &micro_info->ai_addrlen)) < 0){
            printf("recvfrom() failed in micro server");
        }

        clientMessage[strlen(clientMessage)] = '\0';
        printf("Received from indirection server:\n<%s>\n\n", clientMessage);

        //If client ends session get out of loop and close socket
        if(strcmp(clientMessage, "0") == 0){
            done = 1;
        }
        else{
            //Convert CAD to destination currency
            convertCurr(clientMessage, serverMessage);
            sprintf(serverMessage, "%s\n------------Choose a service.-----------\n", serverMessage);
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

void convertCurr(char clientMessage[], char serverMessage[]){
    int amount = 0;
    double newAmount = 0;

    //Get amount of money from client input
    char* start = strchr(clientMessage, '$');

    start += 1;
    char* end = strchr(start, ' ');

    int len = end - start;

    char strAmount[MAX_MESSAGE_LENGTH];
    strncpy(strAmount, start, len);
    strAmount[len] = '\0';

    //Converting string to int
    for(int i = 0; strAmount[i] != '\0'; i++){
        if(!isdigit(strAmount[i])){
            strcpy(serverMessage, "Amount should be an integer.");
            return;
        }
        amount = (strAmount[i] - 48) + (10 * amount);
    }

    //Get destination currency
    char dest[MAX_MESSAGE_LENGTH];
    strcpy(dest, start + len + 5);
    dest[3] = '\0';

    //Convert to destination currency
    if(strcmp(dest, "USD") == 0){
        newAmount = amount * USD;
    }
    else if(strcmp(dest, "EUR") == 0){
        newAmount = amount * EUR;
    }
    else if(strcmp(dest, "GBP") == 0){
        newAmount = amount * GBP;
    }
    else if(strcmp(dest, "BTC") == 0){
        newAmount = amount * BTC;
    }
    else{
        strcpy(serverMessage, "Destination currency is unavailable.\n");
        return;
    }

    //Form message to send to client
    sprintf(serverMessage, "Converted %d CAD to %.2lf %s\n", amount, newAmount, dest);
    serverMessage[strlen(serverMessage)] = '\0';
}