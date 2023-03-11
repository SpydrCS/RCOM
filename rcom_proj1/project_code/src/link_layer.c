// Link layer protocol implementation

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "../include/link_layer.h"

// MISC
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define BUF_SIZE 256

extern bool n;

struct termios oldtio;
struct termios newtio;

LinkLayer ll_info;

int transmission_number=0;

void init_linked_layer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, LinkLayer *ll) {
    if(strcmp((char *) role,"tx") == 0) {
        ll->role = LlTx;
    } else if(strcmp((char *) role,"rx") == 0){
        ll->role = LlRx;
    } else {
        printf(stderr, "Unexpected role: must be tx or rx\n");
        exit(-1);
    }
    strncpy(ll->serialPort, serialPort, strlen(serialPort) + 1);
    
    ll->baudRate = baudRate;
    ll->nRetransmissions = nTries;
    ll->timeout = timeout;
    return;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer ll) {
    LOG_AL_EVENT("State: Opening serial port %s ...\n", ll.serialPort)

    int fd = open(ll.serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    LOG_AL_EVENT("FD1: %d\n", fd)
    if (fd < 0) {
        LOG_AL_ERROR("ERROR of negative fd\n")
        return IO_ERROR;       
    }

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr in llopen (save settings)\n");
        return -1;
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr in llopen (new settings)\n");
        return -1;
    }

    printf("New termios structure set\n");
    LOG_AL_EVENT("FD: %d\n", fd)

    int type = 0;

    if (ll.role == LlTx) {
        type = connect_to_receiver(fd, ll.nRetransmissions, ll.timeout);
        if (type < 0) {
            LOG_AL_ERROR("ERROR: Connect to receiver failed.\n")
            return -1;
        } else {
            LOG_AL_EVENT("SUCCESS: opened connection emitter-receiver (llopen).\n")
        }
    } else {
        type = connect_to_emitter(fd);
        if (type < 0) {
            LOG_AL_ERROR("ERROR: Connect to emitter failed.\n")
            return -1;
        } else {
            LOG_AL_EVENT("SUCCESS: opened connection receiver-emitter (llopen).\n")
        }
    }
    
    ll_info = ll;
    
    LOG_AL_EVENT("llrole: %s\n", ll_info.serialPort)
    return fd;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int number = 0;
int llwrite(unsigned char *buf, int bufSize, int fd)
{
    unsigned char frame[2*PACKET_SIZE + 6] = {0};
    int frameSize = 4;

    frame[0] = FLAG;
    //Set Address
    frame[1] = SET_A_ER;
    //Set Alternating Control Address
    if(number == 0){
        frame[2] = CI(0);
    } else {
        frame[2] = CI(1);
    }
    //Set BCC1
    frame[3] = frame[1]^frame[2];

    unsigned char newPacket[PACKET_SIZE*2+2] = {0};
    //Find BCC2
    unsigned char bcc2 = 0x00;
    for(int i = 0; i < bufSize; i++){
        newPacket[i] = buf[i];
        bcc2 = bcc2 ^ buf[i];
    }

    newPacket[bufSize] = bcc2;

    bufSize = stuff_bytes(newPacket, bufSize+1);
    for (int i = 0; i < 16 && i < bufSize; i++) {
        printf(" %02x ", newPacket[i]);
    }
    printf("\n");


    memcpy (frame+frameSize, newPacket, bufSize);
    frameSize += bufSize;

    frame[frameSize++] = FLAG;
    LOG_AL_EVENT("copy %d\n", frameSize)

    transmission_number = ll_info.nRetransmissions;

    while (true) {

        if (!alarmEnabled) {
            if (transmission_number == 0) {
                LOG_AL_EVENT("Timed out\n")
                return -1;
            }
            send_i_frame(frame, frameSize, fd);
            transmission_number--;
            setup_alarm(ll_info.timeout);
        }

        unsigned char b = 0;
        int ans;
        int r = read(fd, &b, 1);
        if (r > 0) {
            ans = check_i_frame(b, fd, number);
            if (ans == 1) {
                kill_alarm();
                if (number == 0) number = 1; else number = 0;
                if (frameSize < 0) return -1;
                LOG_AL_EVENT("written %d\n", frameSize)
                return frameSize;
            } else if (ans == -1) {
                alarmEnabled = FALSE;
                transmission_number++;
            }
        }
    }
    LOG_AL_ERROR("Unreachable situation\n")
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet, int fd)
{
    printf("=================================\n");
    unsigned char buf[1] = {0};


    int stuffing = FALSE;
    int packetSize = 0;
    unsigned char readPacket[PACKET_SIZE] = {0};

    while (TRUE) {

        int bytes_ = read(fd, &buf, 1);
        if (buf != 0 && bytes_ > -1) {
            int ans = read_frame(buf[0], fd, number);
            switch (ans)
            {
            case -1:
                reset_sm();
                packetSize = 0;
                return -1;
                break;
            case 1:;
                unsigned char bcc2 = readPacket[0];
                for (int i = 1; i < packetSize-1; i++) {
                    bcc2 = (bcc2 ^ readPacket[i]);
                }
                if (bcc2 != readPacket[packetSize-1]) {
                    LOG_AL_ERROR("Data error.\n")
                    reset_sm();
                    packetSize = 0;
                    send_supervision_frame(fd, 0, number);
                    break;
                }
                for (int i = 0; i < packetSize-1; i++) {
                    packet[i] = readPacket[i];
                }
                send_supervision_frame(fd, 1, number);
                if (number == 0) number = 1; else number = 0;
                LOG_AL_EVENT("Information frame received.\n")
                return packetSize-1;
                break;
            case 2:
                if (stuffing) {
                    stuffing = FALSE;
                    readPacket[packetSize++] = buf[0] + 0x20;
                } else {
                    readPacket[packetSize++] = buf[0];
                }
                break;
            case 3:
                stuffing = TRUE;
                break;
            case 5:
                send_supervision_frame(fd, 1, (number == 0) ? 1 : 0);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int fd)
{
    int res;
    if (ll_info.role == LlTx) {
        res = disconnect_from_receiver(transmission_number, ll_info.timeout, fd);
    } else {
        res = disconnect_from_emitter(ll_info.nRetransmissions, ll_info.timeout, fd);
    }

    if (res < 0) {
        LOG_AL_ERROR("ERROR: Disconnection failed in %s %d.\n", (char *) ll_info.role, fd)
    } else {
        LOG_AL_EVENT("Success: Disconnected %d, %s.\n", fd, (char *) ll_info.role)
    }

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        LOG_AL_ERROR("tcsetattr error in llclose %d\n", fd)
        if (close(fd) < 0) {
            LOG_AL_ERROR("ERROR: failed to close %d.\n", fd)
        }
        return -1;
    }

    if (close(fd) < 0) {
        LOG_AL_ERROR("ERROR: failed to close %d.\n", fd)
        return -1;
    }

    LOG_AL_EVENT("Success: closed serial port %s.\n", ll_info.role)

    return 0;
}
