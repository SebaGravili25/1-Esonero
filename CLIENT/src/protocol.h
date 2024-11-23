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

typedef struct{
	char type[1];
	char number[BUFFER_SIZE];
}msg_code;

#endif /* PROTOCOL_H_ */
