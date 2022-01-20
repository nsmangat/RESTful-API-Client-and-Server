//Assignment 3
//Navdeep Mangat 
//CSCN72020
//Due: December 10, 2021
//
//TCP server for assignment 3
//Initial code from Professor Steve Hendrikse, "TCPTimeServer" from https://github.com/ProfessorSteveH/CSCN72020F21/tree/main/Week13
//
//Assumptions:
//      Assume a posting has an attribute for a title ie what it is with author and topic as well as seen from python example ie post 2: 'Another Posting'
//      Assume when a posting is deleted in a collection, the posting collection should be reordered and renumbered ie post 3 deleted in a 4 posting collection,
//      post 4 id becomes 3 but still holds the same info
//
//Notes:
//      Just implmented 1 of the advanced features: GET method filtering
//      Tested client/server using multiple startup projects feature in VS
//      Test data in SavePostings.txt file

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>

#include "NetworkingFunctions.h"

#define LISTENINGPORT   "8080"      //needs to be a string
#define POSTLENGTH 100
#define MAXCOLLECTIONSIZE 10        //change this to adjust max collection size


int main(void)
{

    POST posts[MAXCOLLECTIONSIZE];
    initializePosts(posts);                 //initialize array of posts ie the collection

    char saveFile[] = "SavePostings.txt";   //file to save to/load from

    loadPostings(posts, saveFile);          //if any posts to load, load in first 

    InitializeWindowsSockets();

    printf("Config the local addr...\n");
    struct addrinfo* bind_address = ConfigureLocalAddress(LISTENINGPORT, TCP);

    printf("Creating socket...\n");
    SOCKET socket_listen = CreateBindListeningSocket(bind_address);

    printf("Start listening...\n");
    StartListeningForConnections(socket_listen);

    printf("Waiting for connection...\n");
    SOCKET socket_client = WaitForAndAcceptConnection(socket_listen);

    printf("Client is connected...");
    RecvRequestAndSendResponse(socket_client, posts);

    printf("Closing the connection...\n");
    CloseSocketConnection(socket_client);

    printf("Closing listening socket...\n");
    CloseSocketConnection(socket_listen);

    ShutdownWindowsSockets();

    savePostings(posts, saveFile);                              //when shutting off, save all posts currently in collection

    return 0;
}

