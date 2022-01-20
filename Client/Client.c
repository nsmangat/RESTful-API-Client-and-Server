//Assignment 3
//Navdeep Mangat 
//CSCN72020
//Due: December 10, 2021
//
//TCP client for assignment 3
//Initial code from Professor Steve Hendrikse, "TCPTimeClient" from https://github.com/ProfessorSteveH/CSCN72020F21/tree/main/Week13

#include "NetworkingFunctions.h"
#include <stdio.h>
#include <stdbool.h>


#define MAXBUFFER	1000		//increased buffer size from initial in case of long strings and/or if collection size is modified on server side
#define POSTLENGTH 100			//for various strings to make up a post
#define PARSECHAR 2				//for variables that hold characters that program parses on such as '/' and ' ' ie when using strtok()
#define MAXCOLLECTIONSIZE 10	//collection size, used on client side just to ensure client inputs a number that collection holds for getting a single post for example

int main(void)
{
	// first set up Winsock  (same as server)
	InitializeWindowsSockets();

	printf("Config the remote addr...\n");
	struct addrinfo* peer_address = ConfigureRemoteAddress("127.0.0.1", "8080", TCP);

	printf("Creating socket and connect...\n");
	SOCKET peer_socket = CreateAndConnectRemoteSocket(peer_address);

	//send initial 'greeting'
	//char* message = "get current time\n\0";


	bool whileCondition = true;

	while (whileCondition)														//while loop like in server, so client/server can continue to interact instead of program shutting off
	{																			//after 1 interaction, exit request available to shut down program now

		int input;
		char* message = "";

		displayMenu();															//display menu options to client

		if (scanf_s(" %d", &input) == 1)
		{
			if (!(1 <= input && input <= 7))									//make sure it's valid menu option, or else loop back to start of menu
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");													//clear stdin so won't infinitley loop
				continue;
			}
		}
		else
		{
			printf("Invalid input, please try again\n");
			scanf_s("%*c");
			continue;
		}


		char title[POSTLENGTH];												//used for creating new post, updating and filtering ie postTitle information goes into title
		char author[POSTLENGTH];
		char topic[POSTLENGTH];
		char bufferPost[MAXBUFFER];

		//for case 2										
		int postInput;
		char convertPostInput[3];
		char getPosts[POSTLENGTH] = "GET /POSTS/";

		//for case 3
		char newPostBuffer[POSTLENGTH];

		//for case 4
		int putPosting;

		char titleChoice;
		char authorChoice;
		char topicChoice;
		char defaultString[POSTLENGTH];
		memset(defaultString, '\0', POSTLENGTH);
		strcpy(defaultString, "PUT /");

		char updateString[POSTLENGTH];
		memset(updateString, '\0', POSTLENGTH);

		//for case 5
		char deleteString[POSTLENGTH];

		//for case 6
		char postTitleFilter[POSTLENGTH];
		char authorFilter[POSTLENGTH];
		char topicFilter[POSTLENGTH];

		memset(postTitleFilter, '\0', POSTLENGTH);
		memset(authorFilter, '\0', POSTLENGTH);
		memset(topicFilter, '\0', POSTLENGTH);


		char defaultFilter[POSTLENGTH];
		memset(defaultFilter, '\0', POSTLENGTH);
		strcpy(defaultFilter, "GET /POSTS?");

		char updateFilterString[POSTLENGTH];
		memset(updateFilterString, '\0', POSTLENGTH);

		switch (input)
		{
		case 1:																						//if client wants all posts, simply send this message
			message = "GET /posts ";
			break;
		case 2:
																									//for getting specific post, get post number
			printf("Please input the posting you would like to see. The posting collection holds 10 posts.\n");
			if (scanf_s(" %d", &postInput) == 1)
			{
				if (!(1 <= postInput && postInput <= MAXCOLLECTIONSIZE))
				{
					printf("Invalid input, please try again\n");
					scanf_s("%*c");
					continue;
				}
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}

			sprintf(convertPostInput, "%d", postInput);												//concat to getPosts original string so message is GET /POSTS/NUMBER
			strcat(getPosts, convertPostInput);
			message = getPosts;

			break;
		case 3:
			printf("Please input the title of the new post:\n");									//get all the info for new post from client
			scanf_s("%s", title, POSTLENGTH);
			printf("Please input the author of the new post:\n");
			scanf_s("%s", author, POSTLENGTH);
			printf("Please input the topic of the new post:\n");
			scanf_s("%s", topic, POSTLENGTH);
			printf("Thank you, the new post will attempt to be sent to the server.\n\n");
			scanf_s("%*c");																			//just in case for stdin leftovers

			sprintf(newPostBuffer, "POST /%s/%s/%s/", title, author, topic);						//add the elements to string in newPostBuffer, send this string in required format

			message = newPostBuffer;

			break;
		case 4:
																									//getting client info for updating post
			printf("Please select the posting attributes you would like to update.\n");
			printf("Which post would you like to update? Enter a number from 1-10 corresponding to the desired posting:\n");
			if (scanf_s(" %d", &putPosting) == 1)													//first post number to update
			{
				if (!(1 <= putPosting && putPosting <= MAXCOLLECTIONSIZE))
				{
					printf("Invalid input, please try again\n");
					scanf_s("%*c");
					continue;
				}
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}
			scanf_s("%*c");


			printf("Would you like to update the post title for this post?\nInput 'y' for yes or 'n' for no:\n");		//asking for which attributes to update in that post
			scanf_s(" %c", &titleChoice, 1);
			if (titleChoice == 'y')
			{
				printf("Please input the title of the new post:\n");
				scanf_s("%s", title, POSTLENGTH);
			}
			else if (titleChoice == 'n')																				//if no, then set that attribute variable to default string
			{																											//will be used to compare later, if default then that attribute isn't needed
				strcpy(title, defaultString);
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}

			printf("Would you like to update the author for this post?\nInput 'y' for yes or 'n' for no:\n");
			scanf_s(" %c", &authorChoice, 1);
			if (authorChoice == 'y')
			{
				printf("Please input the author of the new post:\n");
				scanf_s("%s", author, POSTLENGTH);
			}
			else if (authorChoice == 'n')
			{
				strcpy(author, defaultString);
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}

			printf("Would you like to update the topic for this post?\nInput 'y' for yes or 'n' for no:\n");
			scanf_s(" %c", &topicChoice, 1);
			if (topicChoice == 'y')
			{
				printf("Please input the topic of the new post:\n");
				scanf_s("%s", topic, POSTLENGTH);
			}
			else if (topicChoice == 'n')
			{
				strcpy(topic, defaultString);
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}
																					//got all the attributes clients want to update
			strcat(updateString, defaultString);									//need default string so copy it to updateString which will be concat'd until final message
			char convert[5];
			sprintf(convert, "%d", putPosting);										//strcat the post number for the message
			strcat(updateString, convert);
			strcat(updateString, "/");												

			if (strcmp(title, defaultString) != 0)									//ie if != 0, means the title was changed so client wants to update the title
			{
				strcat(updateString, "P/");											//strcat so it's in required format for a PUT on server side
				strcat(updateString, title);
				strcat(updateString, "/");
			}

			if (strcmp(author, defaultString) != 0)									//same thing for checking if other attributes were updated
			{
				strcat(updateString, "A/");
				strcat(updateString, author);
				strcat(updateString, "/");
			}

			if (strcmp(topic, defaultString) != 0)
			{
				strcat(updateString, "T/");
				strcat(updateString, topic);
				strcat(updateString, "/");
			}

			if (strcmp(updateString, defaultString) == 0)							//means client said no to all options when asked what attribute they wanted to update		
			{
				scanf_s("%*c");
				continue;
			}

			message = updateString;

			break;
		case 5:
																														//delete a post 
			printf("Please input the posting you would like to delete. The posting collection can hold 10 posts.\n");
			if (scanf_s(" %d", &postInput) == 1)																		//ask for which post to delete
			{
				if (!(1 <= postInput && postInput <= MAXCOLLECTIONSIZE))
				{
					printf("Invalid input, please try again\n\n");
					scanf_s("%*c");
					continue;
				}
			}
			else
			{
				printf("Invalid input, please try again\n");
				scanf_s("%*c");
				continue;
			}

			sprintf(deleteString, "DELETE /%d/", postInput);															//simple sprintf with the required format and send that
			message = deleteString;

			break;
		case 6:
																														//filter the collection
			printf("Would you like to filter the post collection by post title?\nInput 'y' for yes or 'n' for no:\n");	//first asking what client wants to filter on
			scanf_s(" %c", &titleChoice, 1);
			if (titleChoice == 'y')
			{
				printf("Please input the title you would like to filter the posts by:\n");
				scanf_s("%s", postTitleFilter, POSTLENGTH); 
			}
			else if (titleChoice == 'n')
			{
				printf("No author filter will be used\n");

			}
			else
			{
				printf("Invalid input, please try again\n\n");
				scanf_s("%*c");
				continue;
			}

			printf("Would you like to filter the post collection by author?\nInput 'y' for yes or 'n' for no:\n");			//asking if want to filter on author
			scanf_s(" %c", &authorChoice, 1);
			if (authorChoice == 'y')
			{
				printf("Please input the author you would like to filter the posts by:\n");
				scanf_s("%s", authorFilter, POSTLENGTH);
			}
			else if (authorChoice == 'n')
			{
				printf("No author filter will be used\n");
			}
			else
			{
				printf("Invalid input, please try again\n\n");
				scanf_s("%*c");
				continue;
			}

			printf("Would you like to filter the post collection by topic?\nInput 'y' for yes or 'n' for no:\n");			//asking if want to filter on topic
			scanf_s(" %c", &topicChoice, 1);
			if (topicChoice == 'y')
			{
				printf("Please input the topic you would like to filter the posts by:\n");
				scanf_s("%s", topicFilter, POSTLENGTH);
			}
			else if (topicChoice == 'n')
			{
				printf("No topic filter will be used\n");
			}
			else
			{
				printf("Invalid input, please try again\n\n");
				scanf_s("%*c");
				continue;
			}

			strcpy(updateFilterString, defaultFilter);													//had this in case default string was needed

																										//after getting input from client, must check all combinations of filter
																										//ie does client want to filter using all 3 attributes? only author?

			if (postTitleFilter[0] != '\0' && authorFilter[0] != '\0' && topicFilter[0] != '\0')		//all 3 filters
			{
				strcat(updateFilterString, "posttitle=");												//strcat to get proper format ie for title, postitle=title
				strcat(updateFilterString, postTitleFilter);
				strcat(updateFilterString, "&");														//since it's all 3, requires '&'

				strcat(updateFilterString, "author=");
				strcat(updateFilterString, authorFilter);
				strcat(updateFilterString, "&");

				strcat(updateFilterString, "topic=");
				strcat(updateFilterString, topicFilter);												//since we know it's all 3, this is the last so no '&'
			}

			if (postTitleFilter[0] != '\0' && authorFilter[0] != '\0' && topicFilter[0] == '\0')		//post title and author filter
			{
				strcat(updateFilterString, "posttitle=");												//we know it's 2 attributes to filter on, so just 1 '&' ect.
				strcat(updateFilterString, postTitleFilter);
				strcat(updateFilterString, "&");

				strcat(updateFilterString, "author=");
				strcat(updateFilterString, authorFilter);
			}

			if (postTitleFilter[0] != '\0' && authorFilter[0] == '\0' && topicFilter[0] != '\0')			//post title and topic filter
			{
				strcat(updateFilterString, "posttitle=");
				strcat(updateFilterString, postTitleFilter);
				strcat(updateFilterString, "&");

				strcat(updateFilterString, "topic=");
				strcat(updateFilterString, topicFilter);
			}

			if (postTitleFilter[0] == '\0' && authorFilter[0] != '\0' && topicFilter[0] != '\0')			//author and topic filter
			{
				strcat(updateFilterString, "author=");
				strcat(updateFilterString, authorFilter);
				strcat(updateFilterString, "&");

				strcat(updateFilterString, "topic=");
				strcat(updateFilterString, topicFilter);
			}

			if (postTitleFilter[0] != '\0' && authorFilter[0] == '\0' && topicFilter[0] == '\0')			//just posttitle filter
			{
				strcat(updateFilterString, "posttitle=");
				strcat(updateFilterString, postTitleFilter);
			}

			if (postTitleFilter[0] == '\0' && authorFilter[0] != '\0' && topicFilter[0] == '\0')			//just author filter
			{
				strcat(updateFilterString, "author=");
				strcat(updateFilterString, authorFilter);
			}

			if (postTitleFilter[0] == '\0' && authorFilter[0] == '\0' && topicFilter[0] != '\0')			//just topic filter
			{
				strcat(updateFilterString, "topic=");
				strcat(updateFilterString, topicFilter);
			}

			message = updateFilterString;

			break;
		case 7:																								//when client wants to stop program, send quit condition
																											//also will set while loop condition to false so it'll end
																											//on client side as well
			printf("Preparing to send exit request to shut down the program\n");
			message = "QUIT ";
			whileCondition = false;
			break;
		default:
			return;
		}


		int sent = send(peer_socket, message, strlen(message), 0);
		if (sent == 0)
		{
			fprintf(stderr, "send failed\n");
			exit(1);
		}

		//receive reply from server
		char buffer[MAXBUFFER];
		memset(buffer, '\0', MAXBUFFER);

		while (recv(peer_socket, buffer, MAXBUFFER, 0) != 0)
		{
			printf("%s", buffer);								//had error where when I put whole client in while loop, wouldn't send properly until I put the print in the while loop
			
			break;
		}
	}

	printf("Closing the connection...\n");
	CloseSocketConnection(peer_socket);
	ShutdownWindowsSockets();
}