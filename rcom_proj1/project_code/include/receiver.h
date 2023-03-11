#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <sys/types.h>
#include "packet.h"
#include "connection.h"
#include "utils.h"
#include "link_layer.h"
#include "logs.h"

void receiver(const char *filename, int fd);

int get_file_info(unsigned char packet[], int fd);

int create_file(const char *filename);

int disconnect_receiver(int fd);

#endif
