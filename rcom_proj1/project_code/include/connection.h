#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "link_layer.h"
#include "packet.h"
#include "logs.h"
#include "alarm.h"

#define NUMBER_OF_REPEAT_SETS 1
#define NUMBER_OF_DUPLICATE_MESSAGES 1
#define BIT_FLIP_RATE 0

#define SUCCESS 0
#define NULL_POINTER_ERROR (-1)
#define EOF_DISCONNECT (-2)
#define TOO_MANY_ATTEMPTS (-3)
#define TIMED_OUT (-4)
#define IO_ERROR (-5)
#define INVALID_RESPONSE (-6)
#define BUFFER_OVERFLOW (-7)
#define OUT_OF_ORDER (-8)
#define PARITY_ERROR_1 (-9)
#define PARITY_ERROR_2 (-10)
#define CONFIGURATION_ERROR (-11)
#define WRONG_HEADER (-12)

#define FLAG   0x7E //-> slide 10
#define SET_A_ER 0x03 //comandos enviados pelo Emissor e Respostas enviadas pelo Receptor
#define SET_A_RE 0x01 //comandos enviados pelo Receptor e Respostasenviadas pelo Emissor
#define SET 0x03 //set up -> slide 7
#define UA  0x07 //unnumbered acknowledgment

#define DISC 0x0B
#define CI(n) ((n) << 6)
#define RR(n) (0x05 | ((n) << 7))
#define REJ(n) (0x01 | ((n) << 7))

#define ESC11 0x7D
#define ESC12 0x5E
#define REP1 0x7E

#define ESC21 0x7D
#define ESC22 0x5D
#define REP2 0x7D

#define LL_SIZE_MAX 5192

typedef enum {
    READ_START_FLAG, READ_ADDRESS, READ_CONTROL, READ_BCC, READ_END_FLAG
} state_t;

int stuff_bytes(unsigned char *bytes, int nb);

int read_frame(unsigned char byte, int fd, unsigned int number);
int check_supervision_frame(int fd, LinkLayerRole role, unsigned char first_byte);
int check_i_frame(unsigned char val, int fd, int ca);

void reset_sm();
int send_i_frame(unsigned char frame[], int frameSize, int fd);
int send_supervision_frame(int fd, int t, int number);
int send_set();

int connect_to_receiver(const int fd, const int nTries, const int timeout);
int connect_to_emitter(int fd);

int disconnect_from_receiver(int nTries, int timeout, int fd);
int disconnect_from_emitter(int nTries, int timeout, int fd);

int closeState(unsigned char byte, int fd);
int receiver_await_disc(int fd);
int emitter_send_disc(int fd);
int emitter_send_disc_ua(int fd);
int receiver_send_disc(int fd);
int receiver_await_ua(int fd);
int await_disc(int fd);

#endif // _CONNECTION_H_