#pragma once

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>

#include "NetworkingFunctions.h"

#define LISTENINGPORT   "8080"      //needs to be a string
#define POSTLENGTH 100
#define MAXCOLLECTIONSIZE 10


void initializePosts(POST posts[]);

void savePostings(POST posts[], char fileName[]);
void loadPostings(POST posts[], char fileName[]);

