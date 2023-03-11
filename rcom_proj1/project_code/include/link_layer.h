// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#define FALSE 0
#define TRUE 1

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1


#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alarm.h"
#include "connection.h"
#include "logs.h"

// Fiils linked Layer 
void init_linked_layer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, LinkLayer *ll);

// Open a connection using the "port" parameters defined in struct linkLayer.
int llopen(LinkLayer ll);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(unsigned char *buf, int bufSize, int fd);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char *packet, int fd);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int fd);

#endif // _LINK_LAYER_H_
