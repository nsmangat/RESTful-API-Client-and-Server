//Assignment 3
//Navdeep Mangat 
//CSCN72020
//December 10, 2021
//
//Network common static library 
//Initial library functions from Professor Steve Hendrikse, "NetworkCommonStaticLibrary" from https://github.com/ProfessorSteveH/CSCN72020F21/tree/main/Week13


#pragma once
#define _CRT_SECURE_NO_WARNINGS

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x6000
#endif

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <stdbool.h>


#define DISPLAYBUFFERSIZE	100
#define SENDBUFFERSIZE	1000
#define MAXLISTENERS		10

#define POSTLENGTH 100

typedef struct post                         //Post struct with its members
{
    int id;
    char postTitle[POSTLENGTH];
    char author[POSTLENGTH];
    char topic[POSTLENGTH];
    bool status;                            //status = 0 means empty post, status = 1 means has info

}POST;

typedef enum proto { UDP, TCP } PROTOCOL;

//common
void InitializeWindowsSockets();
void CloseSocketConnection(SOCKET);
void ShutdownWindowsSockets();

//server only
struct addrinfo* ConfigureLocalAddress(char*, PROTOCOL);
SOCKET CreateBindListeningSocket(struct addrinfo*);
void StartListeningForConnections(SOCKET);
SOCKET WaitForAndAcceptConnection(SOCKET);
//void RecvRequestAndSendResponse(SOCKET, POST posts[]);
void RecvUDPRequestAndSendResponse(SOCKET);

//client only
struct addrinfo* ConfigureRemoteAddress(char*, char*, PROTOCOL);
SOCKET CreateAndConnectRemoteSocket(struct addrinfo*);



void getAllOfCollection(SOCKET socket_client, char* buffer, POST posts[]);				                        //returns entire collection
void createSinglePayload(SOCKET socket_client, char* buffer, POST posts[], int postID);		                    //get single post
void createPosting(SOCKET socket_client, char* buffer, POST posts[], char slash[], char requestCopy[]);		    //create new posting
void updatePost(SOCKET socket_client, char* buffer, POST posts[], char slash[], char requestCopy[]);			//PUT to update a posting
void deletePost(SOCKET socket_client, char* buffer, POST posts[], char slash[], char deleteCopy[]);		        //delete a posting in the collection

void getFilter(SOCKET socket_client, char* buffer, POST posts[], char filter[]);							    //advanced feature: GET method filtering

