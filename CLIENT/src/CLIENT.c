/*
 ============================================================================
 Name        : CLIENT.c
 Author      : Gravili Sebastiano
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#define closesocket close

#include <stdio.h>
#include <ctype.h>
#include "protocol.h"

int void_control(char *str);
int format_control(char *str);
int digitLimit_control(char *str);

void clearwinsock() {
	#if defined WIN32
		WSACleanup();
	#endif
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(void) {
		#if defined WIN32
			// Initialize Winsock
			WSADATA wsa_data;
			int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
			if (result != NO_ERROR) {
				printf("Error at WSAStartup()\n");
				return 0;
			}
		#endif

	// create client socket
		int c_socket;
		c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (c_socket < 0) {
			errorhandler("socket creation failed.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

	// set connection settings
		struct sockaddr_in sad;
		memset(&sad, 0, sizeof(sad));
		sad.sin_family = AF_INET;
		sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP del server
		sad.sin_port = htons(PROTO_PORT); // Server port

	// connection
		if (connect(c_socket, (SOCKADDR*) &sad, sizeof(sad)) < 0) {
			errorhandler("Failed to connect.\n");
			#if defined WIN32
				system("pause");
			#endif
			closesocket(c_socket);
			return -1;
		}

	// get data from user
		printf("Enter the type of password to generate followed by the desired length (e.g., n 8 = for a numeric password of 8 characters):\n"
				"n: numeric password (digits only)\n"
				"a: alphabetic password (lowercase letters only)\n"
				"m: mixed password (lowercase letters and numbers)\n"
				"s: secure password (uppercase letters, lowercase letters, numbers, and symbols)\n\n"
				"Enter a length between 6 and 32 characters.\n"
				"Enter 'q' to exit.\n");

		int esc;
		msg_code m;
		/* Do-While loop to continue data input, sending, and receiving between the client and the server
		 * until the termination character 'q' is entered*/
		do{
			esc = 0;
			char *string = (char *)calloc(BUFFER_SIZE, sizeof(char));
			printf("\n=\t");
			fgets(string, BUFFER_SIZE, stdin);
			string[strlen(string) - 1] = '\0';
			if (void_control(string)){
				int format = 0;
				format = format_control(string);
				if(format == 1){
					memset(&m, '\0', sizeof(msg_code));
					strcpy(m.type, strtok(string, " "));
					strcpy(m.number, strtok(NULL, "\0"));
					if(digitLimit_control(m.number)){
						// send data to server
						if (send(c_socket, &m, sizeof(msg_code), 0) != sizeof(msg_code)){
							errorhandler("send() sent a different number of bytes than expected");
							closesocket(c_socket);
							return -1;
						}

						char *password = (char *)calloc(atoi(m.number), sizeof(char *));
						// get data from server
						if ((recv(c_socket, password, BUFFER_SIZE, 0)) <= 0){
							errorhandler("recv() failed or connection closed prematurely");
							closesocket(c_socket);
						}
						printf("Generated Password: %s\n", password);
						free(password);
					}
					else
						errorhandler("INSERT A VALID STRING:: password too short/long, insert a number between 6 and 32\n");
				}
				else if(format == 2){
					esc = 1;
					memset(&m, '\0', sizeof(msg_code));
					strcpy(m.type, string);
					// send data to server
					if (send(c_socket, &m, sizeof(msg_code), 0) != sizeof(msg_code)){
						errorhandler("send() sent a different number of bytes than expected");
						closesocket(c_socket);
						return -1;
					}
					memset(string, '\0', strlen(string));
					// get data from server
					if ((recv(c_socket, string, BUFFER_SIZE, 0)) <= 0){
						errorhandler("recv() failed or connection closed prematurely");
						closesocket(c_socket);
					}
					printf("%s\n\n", string);
					#if defined WIN32
						system("pause");
					#endif
					return 0;
				}
				else if(format == 3)
					errorhandler("INSERT A VALID STRING:: password type doesn't match\n");
				else if(format == 0)
					errorhandler("INSERT A VALID STRING:: string doesn't match the format, insert a string with this format(n 10)\n");
			}
			else{
				errorhandler("INSERT A VALID STRING:: string is empty\n");
			}
				free(string);
			}
			while(esc == 0); // end Do-While

		#if defined WIN32
			system("pause");
		#endif

		return 0;
} // main end

/* Function used to check if a valid and available password type has been entered
 * If the type is valid, it returns 1;
 * If the type is 'q', it returns 2;
 * In any other case, it returns 3. */
int method_control(char method){
	if (method == 'n' || method == 'a' || method == 'm' || method == 's')
		return 1;
	else if(method == 'q')
		return 2;
	else
		return 3;
}

/* Function used to check if the entered string is empty or not
 * If it is NOT empty, it returns 1;
 * If it is empty, it returns 0. */
int void_control(char *str){
	if (strcmp(str, "\0") == 0)
		return 0;
	else
		return 1;
}

/* Function used to check if the string complies with the predefined input format ('type''space''number')
 * If the first character of the string is NOT alphabetical or is recognized as an invalid type, it returns 3;
 * If the first character of the string is alphabetical but is recognized as the termination character 'q', it returns 2;
 * If the first character of the string is alphabetical and is recognized as a valid type, the next check is performed;
 * If the second character of the string is NOT a space, it returns 0;
 * If the second character of the string is a space, it checks whether all characters following the space are numbers.
 * If there are non-numeric characters, it returns 0;
 * If all checks are passed, the function returns 1. */
int format_control(char *str){
	int method = 0;
	if(isalpha(str[0]) == 0 || (method = method_control(str[0])) != 1){
		if(method != 2)
			return 3;
		else
			return 2;
	}
	else if(str[1] != ' ')
		return 0;
	else{
		for(int i = 2; i < strlen(str); i++){
			if(isdigit(str[i]) == 0)
				return 0;
		}
	}
	return 1;
}

/* Function used to check if the length of the password to be created is between 6 and 32, inclusive
 * If the number satisfies this condition, it returns 1;
 * If the number does NOT satisfy this condition, it returns 0. */
int digitLimit_control(char *str){
	int number = atoi(str);
	if(number >= 6 && number <= 32)
		return 1;
	else
		return 0;
}
