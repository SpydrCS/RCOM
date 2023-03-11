// Application layer protocol implementation
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/application_layer.h"
#include "link_layer.h"
#include "emitter.h"
#include "receiver.h"
#include "logs.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename)
{
    init_driver_logs();
    init_ll_logs();
    init_al_logs();

    LinkLayer ll;

    init_linked_layer(serialPort, role, baudRate, nTries, timeout, &ll);

    int fd = llopen(ll);
    LOG_AL_EVENT("FD llopen: %d\n", fd)
    if (fd < 0) {
        LOG_AL_ERROR("ERROR: opening serial port failed (llopen). Couldn't establish connection.\n")
        exit (-3);
    } else {
        LOG_AL_EVENT("Success: opened serial port (llopen). Established connection.\n")
    }

    if(ll.role == LlTx) {
        LOG_AL_EVENT("emitter\n")
        emitter(filename, fd);
    } else {
        LOG_AL_EVENT("receiver\n")
        receiver(filename, fd);
    }
}
