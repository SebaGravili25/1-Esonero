/*
 * protocol.h
 *
 *  Created on: 12 nov 2024
 *      Author: Gravili Sebastiano
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTO_PORT 60000
#define BUFFER_SIZE 64
#define QLEN 5 // Number of maximum client in queue

typedef struct{
	char type[1];
	char number[BUFFER_SIZE];
}msg;

#endif /* PROTOCOL_H_ */
