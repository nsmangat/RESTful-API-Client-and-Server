//Functions for Server.c

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>

#include "NetworkingFunctions.h"
#include "ServerFunctions.h"


#define LISTENINGPORT   "8080"      //needs to be a string
#define POSTLENGTH 100
#define MAXCOLLECTIONSIZE 10

void initializePosts(POST posts[])								//array of posts ie the collection, set IDs for the spots, set status to off since nothing in there until
{																//load or post requests
    for (int i = 0; i < MAXCOLLECTIONSIZE; i++)
    {
        posts[i].id = i + 1;
        posts[i].status = 0;
    }
}

void savePostings(POST posts[], char fileName[])						//write to file when shutting down program, write all posts in collection before shutdown
{
	FILE* fp;
	fp = fopen(fileName, "w+");
	if (fp == NULL)
	{
		printf("Error opening file\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < MAXCOLLECTIONSIZE; i++)							//write each post line by line, appears like "postTitle author topic"
	{
		if (posts[i].status != 0)
		{
			fprintf(fp, "%s %s %s",
				posts[i].postTitle,
				posts[i].author,
				posts[i].topic);
			fprintf(fp, "\n");
		}

	}

	fclose(fp);
}

void loadPostings(POST posts[], char fileName[])
{
	FILE* fp;
	if (fp = fopen(fileName, "r") != NULL)
	{
		fp = fopen(fileName, "r");
		if (fp == NULL)
		{
			printf("Error opening file\n");
			exit(EXIT_FAILURE);
		}
	}

	char temp[POSTLENGTH];													//use temp to hold the lines being read in from file
	int i = 0;
	int count = 1;
	while (fgets(temp, POSTLENGTH, fp) != NULL)
	{
		temp[strcspn(temp, "\n")] = 0;										//get rid of the \n from the line being read in
		char* token = strtok(temp, " ");									//strtok to get string before each space ie this gets first segment (postTitle)
		strcpy(posts[i].postTitle, token);
		while (token != NULL)
		{
			token = strtok(NULL, " ");										//this one gets second segment (author), set it to author in array

			if (count == 1)
			{
				strcpy(posts[i].author, token);
			}
			else if (count == 2)											//count will increase after reading in 1 segment, loops, when getting 3rd token, will be topic segment
			{
				strcpy(posts[i].topic, token);
			}

			count++;
		}

		posts[i].status = 1;												//turn that post index array to on since it has a posting
		i++;																//increment and loop around, next line read in goes into next index
		count = 1;
		memset(temp, "\0", POSTLENGTH);
	}
}
