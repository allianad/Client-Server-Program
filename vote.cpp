/*
Voting Micro Server
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

//Number of votes for each candidate
int nari = 313;
int yuseong = 313;
int dokja = 142;
int medea = 352;
int wyn = 242;

//encryption key
int key = 3;

//voted flag
bool voted = false;

//Shows if client chose to vote
//opt = 0 if client didn't choose to vote, 1 if client chose to vote.
int choseVote = 0;

void showCandidates(char serverMessage[]);
void vote(char clientMessage[], char serverMessage[]);
void showResults(char serverMessage[]);

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

    getaddrinfo(NULL, "12002", &server_info, &micro_info);

    int status;

    //Create a socket
    int microSocket = socket(micro_info->ai_family, micro_info->ai_socktype, micro_info->ai_protocol);
    if(microSocket == -1){
        printf("socket() failed in micro server.");
        exit(1);
    }

    printf("Hello! I am the voting micro server!\n\n");
    printf("Commands:\n3 List candidates.\n4 Vote.\n5 Show results.\n\n");


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
        printf("Received from indirection server:\n<%s>\n\n", clientMessage);

        //If client ends session get out of loop and close socket
        if(strcmp(clientMessage, "0") == 0){
            done = 1;
        }
        else{
            //If client didn't chose to vote previously
            if(choseVote == 0){
                if(strcmp(clientMessage, "3") == 0){
                    showCandidates(serverMessage);
                }

                else if(strcmp(clientMessage, "4") == 0){
                    //If client already voted notify that they can't vote again
                    if(voted == true){
                        strcpy(serverMessage, "Sorry you cannot vote again.");
                    }  
                    else{
                        choseVote = 1;
                        strcpy(serverMessage, "key: 3");
                    }                 
                }

                else if(strcmp(clientMessage, "5") == 0){
                    showResults(serverMessage);
                }

                else{
                    strcpy(serverMessage, "Command is invalid");
                }
            }
            //If client chose to vote already
            else{
                vote(clientMessage, serverMessage);
                choseVote = 0;
            }          

            if(choseVote == 0){
                sprintf(serverMessage, "%s\n\n------------Choose a service.-----------\n", serverMessage);
            }

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

void showCandidates(char serverMessage[]){
    strcpy(serverMessage, "----------------Candidates----------------\n");
    strcat(serverMessage, "First Name\tLast Name\tID\n");
    strcat(serverMessage, "Nari\t\tOh\t\t1111\n");
    strcat(serverMessage, "Yuseong\t\tCheon\t\t2222\n");
    strcat(serverMessage, "Dokja\t\tKim\t\t3333\n");
    strcat(serverMessage, "Medea\t\tSolon\t\t4444\n");
    strcat(serverMessage, "Wyn\t\tWillow\t\t5555\n");
    return;
}

void vote(char clientMessage[], char serverMessage[]){
    int id = 0;

    //Change vote to integer
    for(int i = 0; clientMessage[i] != '\0'; i++){
        if(!isdigit(clientMessage[i])){
            strcpy(serverMessage, "Amount should be an integer.");
            return;
        }
        id = (clientMessage[i] - 48) + (10 * id);
    }

    //Decode vote
    id = id / key;

    //Add vote to results
    if(id == 1111){
        nari++;
    }
    else if(id == 2222){
        yuseong++;
    }
    else if(id == 3333){
        dokja++;
    }
    else if(id == 4444){
        medea++;
    }
    else if(id == 5555){
        wyn++;
    }
    else{
        strcpy(serverMessage, "ID is not available");
        return;
    }

    //Keep track that client has voted
    voted = true;

    //Create message to send back to client
    strcpy(serverMessage, "Thanks! Your vote has been counted. You may now view the results");
    return;
}

void showResults(char serverMessage[]){
    //Check if client already voted
    if(voted == true){
        sprintf(serverMessage,"---------------Voting Results---------------");
        sprintf(serverMessage, "%s\nFirst Name\tLast Name\tID\tID", serverMessage);
        sprintf(serverMessage, "%s\nNari\t\tOh\t\t1111\t%d", serverMessage, nari);
        sprintf(serverMessage, "%s\nYuseong\t\tCheon\t\t2222\t%d", serverMessage, yuseong);
        sprintf(serverMessage, "%s\nDokja\t\tKim\t\t3333\t%d", serverMessage, dokja);
        sprintf(serverMessage, "%s\nMedea\t\tSolon\t\t4444\t%d", serverMessage, medea);
        sprintf(serverMessage, "%s\nWyn\t\tWillow\t\t5555\t%d", serverMessage, wyn);
    }
    else{
        strcpy(serverMessage, "You must vote first to view the results");
        return;
    }

}