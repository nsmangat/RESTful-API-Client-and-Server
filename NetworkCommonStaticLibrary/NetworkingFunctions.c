//Assignment 3
//Navdeep Mangat 
//CSCN72020
//December 10, 2021
//
//Network common static library 
//Initial library functions from Professor Steve Hendrikse, "NetworkCommonStaticLibrary" from https://github.com/ProfessorSteveH/CSCN72020F21/tree/main/Week13

#define _CRT_SECURE_NO_WARNINGS

#include "NetworkingFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define POSTLENGTH 100		//various strings to make up a post
#define PARSECHAR 2			//for variables that hold characters that program parses on such as '/' and ' 
#define MAXCOLLECTIONSIZE 10


#pragma comment(lib, "ws2_32.lib")

void InitializeWindowsSockets()
{
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "failed to initialize network.  Exiting\n");
		exit(1);
	}
}

struct addrinfo* configureAddress(char* host, char* port, PROTOCOL protocol)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;                  // gonna be v4
	if (protocol == TCP)
	{
		hints.ai_socktype = SOCK_STREAM;    // gonna be tcp
		hints.ai_protocol = IPPROTO_TCP;
	}
	else if (protocol == UDP)
	{
		hints.ai_socktype = SOCK_DGRAM;		// gonna be udp
		hints.ai_protocol = IPPROTO_UDP;
	}
	else  //unknown   exit!
	{
		fprintf(stderr, "Unknown protocol selected. Exiting\n");
		exit(EXIT_FAILURE);
	}
	hints.ai_flags = AI_PASSIVE;                    //bind to * address

	struct addrinfo* bind_address;  //no malloc required here.  see note below
	if (!strncmp(host, "*", 1))	// 0 if same!
		getaddrinfo(0, port, &hints, &bind_address);
	else
		getaddrinfo(host, port, &hints, &bind_address);
	//
	// from: https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
	// All information returned by the getaddrinfo function pointed to by the ppResult 
	// parameter is dynamically allocated, including all addrinfo structures, socket address 
	// structures, and canonical host name strings pointed to by addrinfo structures. Memory
	// allocated by a successful call to this function must be released with a subsequent call
	// to freeaddrinfo.
	return bind_address;
}


struct addrinfo* ConfigureLocalAddress(char* port, PROTOCOL protocol)
{
	return configureAddress("*", port, protocol);
}

struct addrinfo* ConfigureRemoteAddress(char* remoteHost, char* remotePort, PROTOCOL protocol)
{
	return configureAddress(remoteHost, remotePort, protocol);
}

SOCKET CreateBindListeningSocket(struct addrinfo* bind_address)
{
	SOCKET socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
		bind_address->ai_protocol);
	if ((socket_listen) == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed, exiting with error (%d)\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
	{
		fprintf(stderr, "bind() failed, exiting with error  (%d)\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(bind_address);  //see note above.  this frees all the allocated mem.
	return socket_listen;
}

SOCKET CreateAndConnectRemoteSocket(struct addrinfo* peer_address)
{
	SOCKET socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if ((socket_peer) == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed, exiting with error (%d)\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen))
	{
		fprintf(stderr, "connect() failed, exiting with error (%d)\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(peer_address);

	return socket_peer;
}

void StartListeningForConnections(SOCKET socket_listen)
{
	if (listen(socket_listen, MAXLISTENERS) < 0)      //allowed to queue up to 10 connections
	{
		fprintf(stderr, "listen() failed, exiting with and error of (%d)\n", WSAGetLastError());
		exit(1);
	}
}


SOCKET WaitForAndAcceptConnection(SOCKET socket_listen)
{
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &client_len);
	if ((socket_client) == INVALID_SOCKET)
	{
		fprintf(stderr, "accept() failed. (%d)\n", WSAGetLastError());
		return 1;
	}

	//opportunity to clean this up!
	char address_buffer[DISPLAYBUFFERSIZE];
	getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer,
		sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	printf("%s\n", address_buffer);

	return socket_client;
}

void createPayload(char* buffer)
{
	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Local time is: ";

	time_t timer;
	time(&timer);
	char* time_msg = ctime(&timer);

	sprintf(buffer, "%s %s\n\0\0", response, time_msg);
}






void getAllOfCollection(SOCKET socket_client, char* buffer, POST posts[])				//returns entire collection
{
	char newBuffer[SENDBUFFERSIZE];

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(newBuffer, "%s", response);

	strcpy(buffer, newBuffer);

	int counter = 0;

	for (int i = 0; i < MAXCOLLECTIONSIZE; i++)											//loop through array of posts
	{
		if (posts[i].status == 1)														//if status is 1, there is a posting there so add it to newbuffer
		{
			
			sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0",posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
			strcat(buffer, newBuffer);																							//concat to buffer, loop until all posts concatted, send to client

			counter++;
		}
	}
	
	if (counter == 0)																	//if counter doesn't increment, there were no posts in the collection
	{
		strcat(buffer, "Sorry, there are currently no posts in the collection\n");
	}

	strcat(buffer, "\n");

}


void createSinglePayload(SOCKET socket_client, char* buffer, POST posts[], int postID)		//get single post
{
	char newBuffer[SENDBUFFERSIZE];

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(newBuffer, "%s", response);


	strcpy(buffer, newBuffer);

	if (posts[postID - 1].status == 1)														//post id is index+1, so if client requests for post 5, want to send posts[4]
	{
		sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0\0", posts[postID - 1].id, posts[postID - 1].postTitle, posts[postID - 1].author, posts[postID - 1].topic);
		strcat(buffer, newBuffer);
	}
	else
	{
		strcat(buffer, "Sorry, a post with that ID does not exist currently.\n");
	}

	strcat(buffer, "\n");
}

void createPosting(SOCKET socket_client, char* buffer, POST posts[], char slash[], char requestCopy[])		//create new posting
{
	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(buffer, "%s", response);

	char* tokenPost;										//for parsing using strtok

	char author[POSTLENGTH];								//hold attributes for new post
	char title[POSTLENGTH];
	char topic[POSTLENGTH];

	tokenPost = strtok(requestCopy, slash);					//from format of message (requestCopy), strtok first time gives postTitle
	strcpy(title, tokenPost);

	int count = 1;

	while (tokenPost != NULL)								//strtok again for 2nd token which is author and again for third token which is topic
	{
		tokenPost = strtok(NULL, slash);

		if (count == 1)
		{
			strcpy(author, tokenPost);
		}
		else if (count == 2)
		{
			strcpy(topic, tokenPost);
		}
		count++;											//need count so on first iteration, author string goes to author variable, then count++, so 3rd token topic string goes to topic variable
	}

	for (int i = 0; i < MAXCOLLECTIONSIZE; i++)				//add the new post to the end of the collection list if there's room
	{
		if (posts[i].status == 0)
		{
			strcpy(posts[i].postTitle, title);
			strcpy(posts[i].author, author);
			strcpy(posts[i].topic, topic);
			posts[i].status = 1;

			strcat(buffer, "Your post has successfully been created and added to the collection!\n\n");

			return;
		}
	}
}

void updatePost(SOCKET socket_client, char* buffer, POST posts[], char slash[], char requestCopy[])				//PUT to update a posting
{
	char newBuffer[SENDBUFFERSIZE];

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(newBuffer, "%s", response);
	strcpy(buffer, newBuffer);


	char* tokenPost;

	char author[POSTLENGTH];
	char title[POSTLENGTH];
	char topic[POSTLENGTH];
	char stringPostNum[POSTLENGTH];
	int updatePostNum;

	memset(author, "\0", POSTLENGTH);
	memset(title, "\0", POSTLENGTH);
	memset(topic, "\0", POSTLENGTH);

	tokenPost = strtok(requestCopy, slash);
	strcpy(stringPostNum, tokenPost);					//first parse through client message, find which post is requested to be updated
	updatePostNum = atoi(stringPostNum);


	if (posts[updatePostNum - 1].status == 1)			//checking if the post they requested to update is not empty
	{
		while (tokenPost != NULL)
		{
			tokenPost = strtok(NULL, slash);			//stop when cannot parse further
			if (tokenPost == NULL)
			{
				break;
			}

			if (strcmp(tokenPost, "P") == 0)			//format is /P/title/A/author ect, p for post title, a for author, t for topic, when parsed and find 1 of these
			{											//then if we strtok once more, we know what information is being read in ie if first strtok gives P, 2nd gives posttitle string
				tokenPost = strtok(NULL, slash);
				memset(posts[updatePostNum - 1].postTitle, "\0", POSTLENGTH);
				strcpy(posts[updatePostNum - 1].postTitle, tokenPost);
				continue;
			}

			if (strcmp(tokenPost, "A") == 0)											//do it until cannot parse anymore ie got all the fields the client wants to update for this post
			{
				tokenPost = strtok(NULL, slash);
				memset(posts[updatePostNum - 1].author, "\0", POSTLENGTH);
				strcpy(posts[updatePostNum - 1].author, tokenPost);
				continue;
			}
			
			if (strcmp(tokenPost, "T") == 0)
			{
				tokenPost = strtok(NULL, slash);
				memset(posts[updatePostNum - 1].topic, "\0", POSTLENGTH);
				strcpy(posts[updatePostNum - 1].topic, tokenPost);
				continue;
			}
		}

		sprintf(newBuffer, "Post %d has been updated!\n\n", updatePostNum);
		strcat(buffer, newBuffer);
	}
	else
	{
		strcat(buffer, "Sorry, a post with that ID does not exist currently.\n\n");
	}
}

void deletePost(SOCKET socket_client, char* buffer, POST posts[], char slash[], char deleteCopy[])		//delete a posting in the collection
{
	char newBuffer[SENDBUFFERSIZE];

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(newBuffer, "%s", response);
	strcpy(buffer, newBuffer);


	char* tokenPost;

	char stringPostNum[POSTLENGTH];
	int updatePostNum;

	tokenPost = strtok(deleteCopy, slash);
	strcpy(stringPostNum, tokenPost);								//same as PUT, find posting number client wants to delete and check if it exists
	updatePostNum = atoi(stringPostNum);

	if (posts[updatePostNum - 1].status == 1)						//if it does, delete its info, turn status to off
	{
		posts[updatePostNum - 1].postTitle[0] = 0;
		posts[updatePostNum - 1].author[0] = 0;
		posts[updatePostNum - 1].topic[0] = 0;
		posts[updatePostNum - 1].status = 0;


		//sort

		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)					//need to reorder (see assumptions in Server.c), loop through collection until empty spot
		{
			int j = i + 1;

			if (posts[i].status == 0 && posts[j].status == 1)		//check if there are any posts after the empty spot, if not then it's end of collection
			{														//if so, move this posting to the empty one, repeat until no "holes" in the collecion

				strcpy(posts[i].postTitle, posts[j].postTitle);
				strcpy(posts[i].author, posts[j].author);
				strcpy(posts[i].topic, posts[j].topic);
				posts[i].status = 1;

				posts[j].postTitle[0] = 0;
				posts[j].author[0] = 0;
				posts[j].topic[0] = 0;
				posts[j].status = 0;

			}
		}

		sprintf(newBuffer, "Post %d has been deleted! The collection has now been resorted.\n\n", updatePostNum);
		strcat(buffer, newBuffer);
	}
	else
	{
		sprintf(newBuffer, "Sorry, post %d does not currently exist in the collection.\n\n", updatePostNum);
		strcat(buffer, newBuffer);
	}
}


void getFilter(SOCKET socket_client, char* buffer, POST posts[], char filter[])							//advanced feature: GET method filtering
{
	char newBuffer[SENDBUFFERSIZE];

	const char* response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n";

	sprintf(newBuffer, "%s", response);
	strcpy(buffer, newBuffer);
																						//need copies of messages throughout for parsing multiple delimiters
	strcat(filter, "&");																//this line is to add & to end of request message to make it easier to parse on '&' delimiter							
	char filter2[POSTLENGTH];
	memset(filter2, '\0', POSTLENGTH);
	char filter3[POSTLENGTH];
	memset(filter3, '\0', POSTLENGTH);
	char filter3Copy[POSTLENGTH];				//for checking if third filter
	memset(filter3Copy, '\0', POSTLENGTH);

	strncpy(filter2, filter, strlen(filter) + 1);

	char question[PARSECHAR] = "?";														//delimiters I'm using for multiple uses of strtok to parse
	char equals[PARSECHAR] = "=";
	char space[PARSECHAR] = " ";
	char ampersand[PARSECHAR] = "&";


	strncpy(filter3, &filter2[11], POSTLENGTH);											//copying request to the point where it's everything after the ? so I only have required info
	strcpy(filter3Copy, filter3);														//need this for checking if third filter requested

	char* tokenamp = strtok(filter3, ampersand);										//parse on & to check first attribute if there are even multiple, will always be at least 1 '&' from strcat at beginning

	char* copytokenamp1 = tokenamp;														//using these copies and appends variables for when I parse and need to parse further on an "=" for example to get the attribute

	char* tokenamp2;
	char* tokenamp3;

	char tokenAmpAppend[POSTLENGTH];
	char tokenamp2Append[POSTLENGTH];
	char tokenamp3Append[POSTLENGTH];

	memset(tokenAmpAppend, '\0', POSTLENGTH);
	memset(tokenamp2Append, '\0', POSTLENGTH);
	memset(tokenamp3Append, '\0', POSTLENGTH);

	char* checkToken1;
	char* checkToken2;
	char* checkToken3;

	char titleFilter[POSTLENGTH];
	char authorFilter[POSTLENGTH];
	char topicFilter[POSTLENGTH];

	memset(titleFilter, '\0', POSTLENGTH);
	memset(authorFilter, '\0', POSTLENGTH);
	memset(topicFilter, '\0', POSTLENGTH);


	int count = 1;


	tokenamp = strtok(NULL, ampersand);							//check if 2nd ampersand ie 2nd attribute

	char copytokenamp2[100];
	memset(copytokenamp2, '\0', 100);
	if (tokenamp != NULL)										//if there is a 2nd attribute, save it in this string
	{
		strcpy(copytokenamp2, tokenamp);

	}


	if (tokenamp == NULL)									//if tokenamp was NULL, means client wants to filter by just 1 attribute  
	{
		strcat(copytokenamp1, "=");							//strcat '=' to make it easier to parse it author=steve to author=steve=
		checkToken1 = strtok(copytokenamp1, equals);		//now use strtok on '=', this will tell the attribute type

		if (strcmp(checkToken1, "posttitle") == 0)			//based on attribute type ie if it's author, strtok once more to get the actual filter information ie 
		{													//if author=steve=, previous strtok (checkToken1) = author, so next strtok on checkToken1 = steve
			checkToken1 = strtok(NULL, equals);				//have the information client wants to filter on, so save it in author 
			strcpy(titleFilter, checkToken1);
		}
		if (strcmp(checkToken1, "author") == 0)
		{
			checkToken1 = strtok(NULL, equals);
			strcpy(authorFilter, checkToken1);
		}
		if (strcmp(checkToken1, "topic") == 0)
		{
			checkToken1 = strtok(NULL, equals);
			strcpy(topicFilter, checkToken1);
		}
	}
	else if (tokenamp != NULL)											//if original tokenamp wasn't NULL on 2nd strtok, means there's at least 1 more attribute to filter on
	{
		strcat(copytokenamp1, "=");										//this is the same as above to find first filter attribute
		checkToken1 = strtok(copytokenamp1, equals);

		if (strcmp(checkToken1, "posttitle") == 0)
		{
			checkToken1 = strtok(NULL, equals);
			strcpy(titleFilter, checkToken1);
		}
		if (strcmp(checkToken1, "author") == 0)
		{
			checkToken1 = strtok(NULL, equals);
			strcpy(authorFilter, checkToken1);

		}
		if (strcmp(checkToken1, "topic") == 0)
		{
			checkToken1 = strtok(NULL, equals);
			strcpy(topicFilter, checkToken1);

		}

		strcat(copytokenamp2, "=");													//repeat to find 2nd filter attribute
		checkToken2 = strtok(copytokenamp2, equals);

		if (strcmp(checkToken2, "posttitle") == 0)
		{
			checkToken2 = strtok(NULL, equals);
			strcpy(titleFilter, checkToken2);
		}
		if (strcmp(checkToken2, "author") == 0)
		{
			checkToken2 = strtok(NULL, equals);
			strcpy(authorFilter, checkToken2);
		}
		if (strcmp(checkToken2, "topic") == 0)
		{
			checkToken2 = strtok(NULL, equals);
			strcpy(topicFilter, checkToken2);
		}

	}
																	//at this point we have 2 elements to filter on (or 1 but code below not relevant if just 1 filter element)
	char* newtokenamp;
	newtokenamp = strtok(filter3Copy, ampersand);					//strtok on original filter information string after the ?, strtok 3 times to check if third element
	newtokenamp = strtok(NULL, ampersand);							
	newtokenamp = strtok(NULL, ampersand);							


	if (newtokenamp != NULL)										//if not null, there is third filter
	{
		char copytokenamp3[POSTLENGTH];
		memset(copytokenamp3, '\0', POSTLENGTH);
		strcpy(copytokenamp3, newtokenamp);

		strcat(copytokenamp3, "=");									//same method of parsing as before
		checkToken3 = strtok(copytokenamp3, equals);
		if (strcmp(checkToken3, "posttitle") == 0)
		{
			checkToken3 = strtok(NULL, equals);
			strcpy(titleFilter, checkToken3);
		}
		if (strcmp(checkToken3, "author") == 0)
		{
			checkToken3 = strtok(NULL, equals);
			strcpy(authorFilter, checkToken3);
		}
		if (strcmp(checkToken3, "topic") == 0)
		{
			checkToken3 = strtok(NULL, equals);
			strcpy(topicFilter, checkToken3);

		}
	}
																						//now we have some combination of filters ie client wants to filter by 
																						//postTitle and author or maybe just topic, need to check each of those combinations
																						//(7 in total), then check if any posts in collection match filter criteria

	if (titleFilter[0] != '\0' && authorFilter[0] != '\0' && topicFilter[0] != '\0')					//3 filters
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)										//ie if 3 filter elements found (client wants to find a post with 3 matching criteria
		{																				//loop through post array until one is found matching filter critiera, then send it
			if (
				(strcmp(posts[i].postTitle, titleFilter) == 0) && 
				(strcmp(posts[i].author, authorFilter) == 0) && 
				(strcmp(posts[i].topic, topicFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}																									//rest of this conditions are similar format, just checks each filter combination
	else if (titleFilter[0] != '\0' && authorFilter[0] != '\0' && topicFilter[0] == '\0')				//title and author filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].postTitle, titleFilter) == 0) &&
				(strcmp(posts[i].author, authorFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
	else if (titleFilter[0] != '\0' && authorFilter[0] == '\0' && topicFilter[0] != '\0')					//title and topic filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].postTitle, titleFilter) == 0) &&
				(strcmp(posts[i].topic, topicFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
	else if (titleFilter[0] == '\0' && authorFilter[0] != '\0' && topicFilter[0] != '\0')					//author and topic filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].author, authorFilter) == 0) &&
				(strcmp(posts[i].topic, topicFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
	else if (titleFilter[0] != '\0' && authorFilter[0] == '\0' && topicFilter[0] == '\0')					//title filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].postTitle, titleFilter) == 0)
				)
				
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
	else if (titleFilter[0] == '\0' && authorFilter[0] != '\0' && topicFilter[0] == '\0')					//author filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].author, authorFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
	else if (titleFilter[0] == '\0' && authorFilter[0] == '\0' && topicFilter[0] != '\0')					//topic filter
	{
		for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
		{
			if (
				(strcmp(posts[i].topic, topicFilter) == 0)
				)
			{
				sprintf(newBuffer, "Post %d. %s, %s, %s\n\0\0\0", posts[i].id, posts[i].postTitle, posts[i].author, posts[i].topic);
				strcat(buffer, newBuffer);
			}
		}
	}
}




void RecvRequestAndSendResponse(SOCKET socket_client, POST posts[])
{
	bool whileCondition = true;													//created while loop in server (and client) so multiple sends and receives can be done
																				//instead of program shutting off after one client/server interaction
	while (whileCondition)
	{
		printf("Reading request...\n");
		char request[SENDBUFFERSIZE];
		memset(request, '\0', SENDBUFFERSIZE);
		int bytes_received = recv(socket_client, request, SENDBUFFERSIZE, 0);
		printf("Received %d bytes: \n", bytes_received);
		printf("%.*s\n", bytes_received, request);      //the %.*s makes sure we print exactly as many chars as was received (regardless of null termination)

		printf("building response\n");
		char buffer[SENDBUFFERSIZE];


		//parse

		char space[PARSECHAR] = " ";
		char slash[PARSECHAR] = "/";
		char num[PARSECHAR];
		char requestCopy[POSTLENGTH];
		strncpy(requestCopy, &request[5], strlen(request));					//keep a copy of a GET request before strtok just in case

		char deleteCopy[POSTLENGTH];										//copy of DELETE request
		strncpy(deleteCopy, &request[8], strlen(request));

		char filterCopy[POSTLENGTH];
		memset(filterCopy, '\0', POSTLENGTH);
		strcpy(filterCopy, request);										//copy of filter request before strtok
		//filterCopy[strcspn(filterCopy, "\n")] = 0;						//for testing filter from WSL

		char* token = strtok(request, space);								//strtok request to see what time of request it is, first space should tell

		if (strcmp(token, "GET") == 0)														//if it's a GET request, going to be collection, single, or filter
		{
			//"GET /posts "		

			if (request[10] == ' ')															//collection
			{
				getAllOfCollection(socket_client, buffer, posts);
			}
			else if (request[10] == '?')													//filter
			{
				//example - GET /POSTS?posttitle=conestoga&author=steve

				getFilter(socket_client, buffer, posts, filterCopy);
			}
			else
			{																				//single
				//example - GET /POSTS/5

				strncpy(num, &request[11], 2);
				int numRequest = atoi(num);													//convert the post the client wants from char to int 
				createSinglePayload(socket_client, buffer, posts, numRequest);
			}
		}
		else if (strcmp(token, "POST") == 0)												//POST request
		{
			//example - POST /POSTTITLE/AUTHOR/TOPIC

			createPosting(socket_client, buffer, posts, slash, requestCopy);
		}
		else if (strcmp(token, "PUT") == 0)													//PUT request
		{
			//example - PUT /2/P/TITLE/A/AUTHOR/T/TOPIC/ OR /4/P/TITLE/TOPIC

			updatePost(socket_client, buffer, posts, slash, requestCopy);
		}
		else if (strcmp(token, "DELETE") == 0)												//DELETE request
		{
			//example - DELETE /2/

			deletePost(socket_client, buffer, posts, slash, deleteCopy);

		}
		else if (strcmp(token, "QUIT") == 0)												//if client sends request to quit, exit while loop and shut down program
		{																					//format is "QUIT " so that parse on space will tell it's QUIT
			whileCondition = false;
			sprintf(buffer, "The server will prepare to shutdown at the client's request.\n\n");
		}

	printf("Sending response...\n");														//what to send back to client after performing requested operation
	int bytes_sent = send(socket_client, buffer, strlen(buffer), 0);
	printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(buffer));
																							//if no quit, loop back to wait for another request from client 
	}
	
}

void RecvUDPRequestAndSendResponse(SOCKET listen_socket)
{
	struct sockaddr_in cliaddr;
	char buffer[SENDBUFFERSIZE];

	int clientLength = sizeof(cliaddr);  //len is value/resuslt

	int bytesReceived = recvfrom(listen_socket,
		buffer,
		SENDBUFFERSIZE,
		0,
		(struct sockaddr*)&cliaddr,
		&clientLength);
	buffer[bytesReceived] = '\0';	// no guarantee the payload will be NULL terminated
	printf("\nClient sent : %s\n", buffer);

	createPayload(buffer);

	sendto(listen_socket,
		buffer,
		strlen(buffer),
		0,
		(const struct sockaddr*)&cliaddr,
		clientLength);
	printf("Response message sent.\n");
}

void CloseSocketConnection(SOCKET this_socket)
{
	shutdown(this_socket, SD_BOTH);
	closesocket(this_socket);
}

void ShutdownWindowsSockets()
{
	WSACleanup();
}
