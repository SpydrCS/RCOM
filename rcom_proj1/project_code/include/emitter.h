#ifndef _EMITTER_H_
#define _EMITTER_H_

#include "packet.h"
#include "connection.h"
#include "utils.h"
#include "link_layer.h"
#include "logs.h"

void emitter(const char *filename, int fd);

int check_file(const char *filename);

int send_file_info(ctrl_packet *packet, unsigned char *pa, int fd);

int send_data(int fd, int fd_file, unsigned char *pa, unsigned char size, const ctrl_packet *packet);

int send_end_packet(ctrl_packet *packet, unsigned char *pa, int fd);

#endif
