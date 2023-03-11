#ifndef _PACKET_H_
#define _PACKET_H_

#include <sys/types.h>
#include <stdbool.h>

#include "logs.h"

#define C_DATA 1
#define C_START 2
#define C_END 3

#define T_FILE_SIZE 0 //slide 23
#define T_FILE_NAME 1 //slide 23

#define PACKET_SIZE 256
#define BUFFER_SIZE 128

typedef struct {
    char *file_name;
    size_t file_size;
} ctrl_packet;

int process_control_packet(const unsigned char *pa, size_t nb, ctrl_packet *cpacket, bool is_start);
int process_data_packet(unsigned char *pa, unsigned int *size);

int control_packet(char *file_name, size_t file_size, bool is_start, unsigned char *pa);
int assemble_data_packet(unsigned char *pa, int i, int counter);

#endif
