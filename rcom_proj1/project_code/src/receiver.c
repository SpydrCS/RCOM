#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "../include/receiver.h"

void receiver(const char *filename, int fd) {

    LOG_AL_EVENT("[receiver]: started using serial port\n")
    int received = FALSE;
    int *file_write = -1;
    while (!received) {
        unsigned char packet[PACKET_SIZE] = {0};

        int packetsize;
        if ((packetsize = get_file_info(packet, fd)) < 0) { //reads start packet
            LOG_AL_ERROR("ERROR: get_file_info() in receiver.c with r=%d\n", packetsize)
            llclose(fd);
            exit(packetsize);
        }

        int size = 0;
        int r = process_data_packet(packet, &size);
        printf("Data (size = %d): ", size);
        for (int i = 0; packet != NULL && (i < 16 && i < size); i++) {
            printf("%02X ", packet[i]);
        }
        printf("\n");

        if (r == 0) {
            LOG_AL_ERROR("ERROR: process_data_packet() in receiver.c with r=%d\n", r)
            llclose(fd);
            exit(r);
        } else if (r == 1) {
            if (file_write == -1) {
                LOG_AL_ERROR("ERROR: couldnt start to write file.\n")
                return;
            } 
            for (int j = 4; j < size + 4; j++) { 
                fputc(packet[j], file_write);
            }
            continue;
        } else if (r == 2) {
            file_write = fopen(filename, "w+");
            continue;
        } else if (r == 3) {
            fclose(file_write);
            received = TRUE;
            continue;
        }
        if (llclose(fd) < 0) {
            LOG_AL_ERROR("ERROR: Closing connection failed.\n")
            return;
        }
    }
}

//get control packet - start
int get_file_info(unsigned char packet[], int fd) {
    LOG_AL_EVENT("[receiver]: getting file info\n")
    int r = llread(packet, fd);
    LOG_AL_EVENT("R: %d\n", r)
    if (r < 0) {
        LOG_AL_ERROR("[receiver]: error getting file info\n")
    } else {
        LOG_AL_EVENT("[receiver]: got file info\n")
    }
    return r;
}

