/*
 ============================================================================
 Name        : SERVER.c
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
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

char* generate_numeric(size_t lenght);
char* generate_alpha(size_t lenght);
char* generate_mixed(size_t lenght);
char* generate_sicure(size_t lenght);

void errorhandler(char *errorMessage){
	printf("%s", errorMessage);
}

void clearwinsock() {
	#if defined WIN32
		WSACleanup();
	#endif
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

	//create welcome socket
		int my_socket;
		my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(my_socket < 0){
			errorhandler("socket creation failed.\n");
			clearwinsock();
			return -1;
		}

	//set connection setting
		struct sockaddr_in sad;
		memset(&sad, 0, sizeof(sad));
		sad.sin_family = AF_INET;
		sad.sin_addr.s_addr = inet_addr("127.0.0.1");
		sad.sin_port = htons(PROTO_PORT);
		if(bind(my_socket, (SOCKADDR*) &sad, sizeof(sad)) < 0){
			errorhandler("bind() failed.\n");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

	//listen
		int queue;

		queue = listen(my_socket, QLEN);
		if (queue != 0) {
			errorhandler("listen() failed.\n");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

	// accept new connection
		struct sockaddr_in cad; // structure for the client address
		int client_socket;       // socket descriptor for the client
		int client_len;          // the size of the client address
		printf("Waiting for a client to connect...\n\n");
		while(1){
			client_len = sizeof(cad); // set the size of the client address
			if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
				errorhandler("accept() failed.\n");
				// close connection
				closesocket(client_socket);
				clearwinsock();
				return 0;
			}

			printf("New connection from %s:%d\n\n", inet_ntoa(cad.sin_addr), cad.sin_port);

			// get msg from client
			do{
				msg m;

				if (recv(client_socket, &m, sizeof(msg), 0) <= 0) {
					errorhandler("\nConnection closed:: Data reception failed or connection closed prematurely\n\n");
					closesocket(client_socket);
					break;
				}
				char* result = (char *) calloc(atoi(m.number), sizeof(char *));
				if (m.type[0] != 'q'){
					srand(time(NULL));
					if (m.type[0] == 'n')
						result = generate_numeric(atoi(m.number));
					else if (m.type[0] == 'a')
						result = generate_alpha(atoi(m.number));
					else if (m.type[0] == 'm')
						result = generate_mixed(atoi(m.number));
					else if (m.type[0] == 's')
						result = generate_sicure(atoi(m.number));

					// send password to client
					if(send(client_socket, result, strlen(result), 0) != strlen(result)){
						errorhandler("send() sent a different number of bytes than expected");
						closesocket(client_socket);
						clearwinsock();
						break;
					}

					printf("\tPassword generation successful\n");
					free(result);
				}
				else{
					result = "Connection Closed...";
					// send "Connection Closed..." to client
					if(send(client_socket, result, strlen(result), 0) != strlen(result)){
						errorhandler("send() sent a different number of bytes than expected");
						closesocket(client_socket);
						clearwinsock();
						break;
					}
					printf("\nConnection from %s:%d CLOSED\n\n", inet_ntoa(cad.sin_addr), cad.sin_port);
					closesocket(client_socket);
					break;
				}
			}
			while(1);
		}

		closesocket(my_socket);
		clearwinsock();

		return 0;
} // main end

/* Function that counts the occurrences of each character before it is added to the final array.
 * This ensures the creation of passwords with the minimum number of occurrences for each generated character.
 * It returns the number of occurrences present in the array, including the one passed as a reference,
 * as it may already be part of the array. */
int linear_search_char(char vector[], char num){
	int count = 1;
	for (int i = 0; i < strlen(vector); i++){
		if (vector[i] == num){
			count++;
		}
	}
	return count;
}

/* Function that randomly generates a number from 0 to 9 and returns it as a character.
 * The generation uses ASCII codes to enable the rand function, which randomizes only integer numbers. */
char random_digit(){
	return (char)(rand() % 10) + '0';
}

/* Function that randomly generates a lowercase letter from 'a' to 'z' and returns it as a character.
 * The generation uses ASCII codes to enable the rand function, which randomizes only integer numbers. */
char random_lowercase(){
	return (char)(rand() % 26) + 'a';
}

/* Function that randomly generates an uppercase letter from A to Z and returns it as a character.
 * The generation uses the ASCII code to enable the random function, which operates only on integers. */
char random_uppercase(){
	return (char)(rand() % 26) + 'A';
}

/* Function that randomly generates a symbol from those available in an internal array
 * and returns it as a character. The generation uses ASCII codes to enable the randomization process,
 * as rand can only randomize integers. */
char random_symbol(){
	char symbol[] = {'!', '"', '#', '$', '%', '&', '(', ')', '*', '+', '-', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '{', '|', '}'};
	return symbol[(rand() % strlen(symbol))];
}

/* Function that generates and returns a mixed password by randomly selecting either a number.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_numeric(size_t lenght){
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				R_temp = random_digit();
				temp[i] = R_temp;
			}
			else{
				do{
					R_temp = random_digit();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			R_temp = random_digit();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a mixed password by randomly selecting either a lowercase letter.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_alpha(size_t lenght){
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				R_temp = random_lowercase();
				temp[i] = R_temp;
			}
			else{
				do{
					R_temp = random_lowercase();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			R_temp = random_lowercase();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a mixed password by randomly selecting either a number
 * or a lowercase letter.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_mixed(size_t lenght){
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		int version = (rand() % 2);
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				temp[i] = R_temp;
			}
			else{
				do{
					if (version == 0) R_temp = random_lowercase();
					else if(version == 1) R_temp = random_digit();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			if (version == 0) R_temp = random_lowercase();
			else if(version == 1) R_temp = random_digit();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}

/* Function that generates and returns a secure password by randomly selecting a number,
 * a lowercase letter, an uppercase letter, or a symbol.
 * Each step generates a different character, stores it in R_temp, which is then saved into an array.
 * At the end of the function, this array is copied into a pointer, which is returned outside the function. */
char* generate_sicure(size_t lenght){
	char temp[lenght];
	char R_temp;
	char* ris = (char *) calloc(lenght, sizeof(char *));
	for (int i = 0; i < lenght; i++){
		int version = (rand() % 4);
		/*Numbers, generated randomly and converted into characters, are added to a password only if no digit occurs more than twice;
		 * otherwise, the generation is repeated. This rule applies up to a maximum of 20 numbers: a string with digits 0–9 repeated twice each reaches 20 characters.
		 * Beyond this limit, adding another number would create an infinite loop, as a third occurrence of one of the digits would become unavoidable.*/
		if (lenght < 21 && lenght > 0){
			if (i == 0) {
				if (version == 0) R_temp = random_lowercase();
				else if(version == 1) R_temp = random_digit();
				else if(version == 2) R_temp = random_uppercase();
				else if(version == 3) R_temp = random_symbol();
				temp[i] = R_temp;
			}
			else{
				do{
					if (version == 0) R_temp = random_lowercase();
					else if(version == 1) R_temp = random_digit();
					else if(version == 2) R_temp = random_uppercase();
					else if(version == 3) R_temp = random_symbol();
				}
				while(linear_search_char(temp, R_temp) > 2);

				temp[i] = R_temp;
			}
		}
		else{
			if (version == 0) R_temp = random_lowercase();
			else if(version == 1) R_temp = random_digit();
			else if(version == 2) R_temp = random_uppercase();
			else if(version == 3) R_temp = random_symbol();
			temp[i] = R_temp;
		}
	}
	strncpy(ris, temp, lenght);

	return ris;
}



