#include "ClientFunctions.h"
#include "NetworkingFunctions.h"
#include <stdio.h>
#include <stdbool.h>

void displayMenu()																		//display menu options for client to select from
{
	printf("1. GET /posts\n");
	printf("2. GET /posts/?\n");
	printf("3. POST\n");
	printf("4. PUT\n");
	printf("5. DELETE\n");
	printf("6. FILTER GET\n");
	printf("7. Exit program\n\n");

	printf("Please input the corresponding menu number to select a method:\n");
}