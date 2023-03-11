#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/emitter.h"

extern bool n;

/**
 * @brief Transmitter.
 * 1) Check if file exists and opens
 * @param filename 
 */
void emitter(const char *filename, int fd) {
    LOG_AL_EVENT("[emitter]: started using serial port\n")

    int fd_file = check_file(filename); //will be used to read data from file
    if (fd_file < 0) {
        LOG_AL_ERROR("[emitter]: Couldn't open %s\n", filename)
        exit(fd_file);
    }
    
    ctrl_packet packet;
    if ((packet.file_size = get_file_size(filename)) < 0) {
        LOG_AL_ERROR("ERROR: stat(filename, &st) in file utils.c\n")
        exit(-1);
    }
    packet.file_name = (char *) filename;
    LOG_AL_EVENT("[emitter]: Source file: %s with %d bytes. Ok!\n", filename, packet.file_size)

    unsigned char pa[PACKET_SIZE] = {0};

    int r = send_file_info(&packet, pa, fd);
    if (r < 0) {
        LOG_AL_ERROR("ERROR: send_file_info() in emitter.c\n")
        llclose(fd_file);
        exit(r);
    }
    LOG_AL_EVENT("[emitter]: sending data\n")

    if ((r = send_data(fd, fd_file, pa, PACKET_SIZE-4, &packet)) < 0) exit(r);

    if ((r = send_end_packet(&packet, pa, fd)) < 0)  {
        llclose(fd_file);
        exit(r);
    }

    LOG_AL_EVENT("[emitter]: SUCESS => r=%d\n", r)
    return;
}

int send_end_packet(ctrl_packet *packet, unsigned char *pa, int fd) {
    LOG_AL_EVENT("[emitter]: creating end control packet\n");
    int r = control_packet(packet->file_name, packet->file_size, false, pa);

    printf("=================================\n");
    printf("sending end packet (size = %d): \n", packet->file_size);
    if (r < 0 || llwrite(pa, r, fd) < 0) {
        LOG_AL_ERROR("[emitter]: error sending end control packet\n")
        return -1;
    } else {
        LOG_AL_EVENT("[emitter]: sent end control packet\n")
    }
    return 0;
}

int send_data(int fd, int fd_file, unsigned char *pa, unsigned char size, const ctrl_packet *packet) {
    int counter = 0;
    int countBytes = 0;
    int bytesRead = 0;
    while ((bytesRead = read(fd_file, pa, size)) > 0) {

        printf("=================================\n");
        printf("sending data (size = %d): \n", bytesRead);

        counter++;
        countBytes += bytesRead;

        LOG_AL_EVENT("Information frame sent, %d\n", size)

        if (bytesRead <= 0) {
            LOG_AL_ERROR("[emitter]: error reading from file\n");
            return -1;
        } else {
            LOG_AL_EVENT("[emitter]: read %zu bytes from file\n", countBytes)
        }
        bytesRead = assemble_data_packet(pa, bytesRead, counter);

        if (llwrite(pa, bytesRead, fd) < 0) {
            LOG_AL_ERROR("[emitter]: error sending data packet : llwrite\n")
            llclose(fd);
            return -1;
        } else {
            LOG_AL_EVENT("[emitter]: sent packet %d/%d\n", bytesRead, counter)
            printf("sent Ok: %d / %d (%d %%)\n", countBytes, packet->file_size, countBytes*100/packet->file_size);
        }
    }
    return 0;
}

//sends info of file through the port, using llwrite
int send_file_info(ctrl_packet *packet, unsigned char *pa, int fd) {
    LOG_AL_EVENT("[emitter]: sending file info\n")
    LOG_AL_EVENT("PACKETNAME: %s with %d\n", packet->file_name, packet->file_size)
    int n = control_packet(packet->file_name, packet->file_size, true, pa);
    LOG_AL_EVENT("control packet: %d\n", n)
    printf("=================================\n");
    printf("sending start packet (size = %d): \n", packet->file_size);

    int a = llwrite(pa, n, fd);
    LOG_AL_EVENT("chars a: %d\n", a)

    if (n < 0 || a <= 0) {
        LOG_AL_ERROR("[emitter]: error sending file info\n")
        return -1;
    } else {
        LOG_AL_EVENT("[emitter]: sent file info\n")
    }
    return 0;
}

/**
 * @brief Checks if file exists and opens file
 * 
 * @param path of the file
 * @return int 
 */
int check_file(const char *filename) {
    LOG_AL_EVENT("[emitter]: checking input file\n")

    int fd_file = open(filename, O_RDONLY); //read only

    if (fd_file < 0) { //== -1
        LOG_AL_ERROR("[emitter]: error opening input file\n")
        llclose(fd_file);
    } else {
        LOG_AL_EVENT("[emitter]: input file ok\n")
    }

    return fd_file;
}

