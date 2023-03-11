#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/connection.h"
#include "../include/packet.h"

int process_control_packet(const unsigned char *pa, size_t nb, ctrl_packet *cpacket, bool is_start) {
    if (pa == NULL || cpacket == NULL) { 
        LOG_AL_ERROR("ERROR: process_control_packet() in packet.c due to null pointer use\n")
        return NULL_POINTER_ERROR; 
    } else if ((is_start && (pa[0] != C_START)) || (!is_start && (pa[0] != C_END))) {
        LOG_AL_ERROR("ERROR: process_control_packet() in packet.c due to invalid response\n")
        return INVALID_RESPONSE; 
    }
    memset(cpacket, 0, sizeof(*cpacket));
    unsigned char t, l;
    size_t i = 1;

    while (i < nb) {
        t = pa[i++];
        l = pa[i++];

        if (t == T_FILE_SIZE) {
            memcpy((size_t *) cpacket->file_size, pa + i, l);
        } else if (t == T_FILE_NAME) {
            char *s = malloc(l);
            if (s == NULL) return BUFFER_OVERFLOW;
            else memcpy(s, pa + i, l);
            LOG_AL_EVENT("Attributing to packet the file name: %s\n", s)
            cpacket->file_name = s;
        } else {}
        i += l;
    }
    return SUCCESS;
}

int process_data_packet(unsigned char *pa, unsigned int *size) {
    unsigned int counter = 0;
    unsigned int nameSize = 0;
    unsigned int sizeSize = 0;

    switch (pa[0])
    {
    case C_START:
        if (pa[1] == T_FILE_SIZE) {
            sizeSize = pa[2];
            unsigned int newSize = 0;
            int i = 0;
            for (i = 0; i < sizeSize; i++) {
                newSize |= pa[(3+i)] << 8*(sizeSize-i-1); 
            }
            (*size) = newSize;
            i += 3;
            if (pa[i] != T_FILE_NAME) {
                LOG_AL_ERROR("ERROR: not name\n")
                return 0;
            }
            nameSize = pa[(i+1) * 8];

            for (int j = 0; j < nameSize; j++) {
                pa[j] = pa[(i+2+j)];
            }
            pa[nameSize] = '\0';

        } else if (pa[1] == T_FILE_NAME) {
            unsigned int newSize = 0;
            nameSize = pa[2];

            sizeSize = pa[(4+nameSize)];

            for (int i = 0; i < sizeSize; i++) {
                newSize |= pa[(5+nameSize+i)] << 8*(sizeSize-i-1); 
            }
            (*size) = newSize;

            for (int j = 0; j < nameSize; j++) {
                pa[j] = pa[(3+j)];
            }
            pa[nameSize] = '\0';
        }
        return 2;
    case C_END:
        if (pa[1] == T_FILE_SIZE) {
            sizeSize = pa[2];
            unsigned int newSize = 0;
            int i = 0;
            for (i = 0; i < sizeSize; i++) {
                newSize |= pa[(3+i)] << 8*(sizeSize-i-1); 
            }
            (*size) = newSize;
            i += 3;
            if (pa[i] != T_FILE_NAME) {
                LOG_AL_ERROR("ERROR: not name\n")
                return 0;
            }
            nameSize = pa[(i+1) * 8];

            for (int j = 0; j < nameSize; j++) {
                pa[j] = pa[(i+2+j)];
            }
            pa[nameSize] = '\0';

        } else if (pa[1] == T_FILE_NAME) {
            unsigned int newSize = 0;
            nameSize = pa[2];

            sizeSize = pa[(4+nameSize)];

            for (int i = 0; i < sizeSize; i++) {
                newSize |= pa[(5+nameSize+i)] << 8*(sizeSize-i-1); 
            }
            (*size) = newSize;

            for (int j = 0; j < nameSize; j++) {
                pa[j] = pa[(3+j)];
            }
            pa[nameSize] = '\0';
        }
        return 3;
    case C_DATA:
        counter = pa[1];
        (*size) = pa[2] * 256 + pa[3];
        return 1;
    case DISC:
        return 4;
    default:
        return 0;
    }
}

//fill control packet with its different parts about file info (bool is_start=False => is end packet)
int control_packet(char *file_name, size_t file_size, bool is_start, unsigned char *pa) {
 
    unsigned int filenameSize = strlen(file_name);

    unsigned int bytesForFilenameSize = filenameSize/256;
    if (filenameSize - (bytesForFilenameSize * 256) > 0) {
        bytesForFilenameSize++;
    }

    if(bytesForFilenameSize > 255){
        LOG_AL_ERROR("\nGetControlPacket error: filename size bigger than one byte\n")
        return -1;
    }
    
    unsigned char hexaSize[PACKET_SIZE] = {0};
    
    
    sprintf(hexaSize, "%02lX", file_size);

    unsigned int bytesForFileSize = strlen(hexaSize);

    if (bytesForFileSize % 2) bytesForFileSize++;

    bytesForFileSize = bytesForFileSize/2;

    if(bytesForFileSize > 255){
        LOG_AL_ERROR("\nGetControlPacket error: bytes required to represent file size bigger than one\n")
        return -1;
    }

    int index = 0;

    if (is_start == TRUE) {
        pa[index++] = C_START;
    } else {
        pa[index++] = C_END;
    }

    pa[index++] = T_FILE_NAME;
    pa[index++] = filenameSize;

    for(int i=0; i<filenameSize; i++){
        pa[index++] = file_name[i];
    }

    pa[index++] = T_FILE_SIZE;
    pa[index++] = bytesForFileSize;

    for(int i=(bytesForFileSize-1); i>-1; i--){
		pa[index++] = file_size >> (8*i);
	}    
    LOG_AL_EVENT("index %d\n", index)
    LOG_AL_EVENT("writing filename size %d bytes for file size: %d \n", bytesForFilenameSize, bytesForFileSize)

    return index;
}

int assemble_data_packet(unsigned char *pa, int size_data, int counter) {

    size_t j = 0;
    unsigned char dest[PACKET_SIZE] = {0};

    dest[j++] = C_DATA;
    dest[j++] = counter % 255;
    dest[j++] = size_data / 256;
    dest[j++] = size_data % 256;

    for (int k = 0; k < size_data; k++) {
        dest[4+k] = pa[k];
    }
    for (int k = 0; k < (size_data+4); k++) {
        pa[k] = dest[k];
    }

    return size_data + 4;
}