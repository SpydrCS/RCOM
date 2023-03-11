#include "../include/connection.h"

bool n = 0;

state_t state = READ_START_FLAG;
extern int transmission_number;
unsigned char buffer[PACKET_SIZE] = {0};

int stuff_bytes(unsigned char *bytes, int nb) {
    unsigned char d[PACKET_SIZE*2+2] = {0};
    int j = 0;
    for (int i = 0; i < nb; i++) {
        if (bytes[i] == REP1) {
            d[j++] = ESC11;
            d[j++] = ESC12;
        } else if (bytes[i] == REP2) {
            d[j++] = ESC21;
            d[j++] = ESC22;
        } else {
            d[j++] = bytes[i];
        }
    }
    for (int i = 0; i < j; i++) bytes[i] = d[i];

    return j;
}

int data = 0;
int data_pointer = 0;
int d[PACKET_SIZE] = {0};

void reset_sm() {
    data = 0;
    data_pointer = 0;
}

int read_frame(unsigned char byte, int fd, unsigned int ca) {
    while (TRUE) {
        switch (data)
        {
            case 0:
                if (byte == 0x7E) {
                    data = 1;
                    d[data_pointer++] = byte;
                    return 0;
                }   
            case 1:
                if (byte == SET_A_ER) {
                    data = 2;
                    d[data_pointer++] = byte;
                    return 0;
                }
            case 2:
                if (byte != FLAG) {
                    data = 3;
                    d[data_pointer++] = byte;
                    if (!((byte == CI(0) && ca == 0) || (byte == CI(1) && ca == 1))) {
                        reset_sm();
                        return 5;
                    }
                    return 0;
                }
            case 3: 
                if (byte == (d[2] ^ d[1])) {
                    data = 4;
                    d[data_pointer++] = byte;
                    return 0;
                } else if (byte == FLAG){
                    data = 1;
                    data_pointer = 0;
                    return 0;
                } else {
                    LOG_AL_ERROR("Protocol error. \n")
                    data_pointer = 0;
                    reset_sm();
                    return 0;
                }
                break;
            case 4:
                if (byte == 0x7D) {
                    return 3;
                } else if (byte == 0x7E) {
                    data = 0;
                    data_pointer = 0;
                    return 1;
                } else {
                    return 2;
                }
                break;
        }
        return 0;
    }
}    

int i_pos = 0;
unsigned char temp[PACKET_SIZE] = {};

int check_supervision_frame(int fd, LinkLayerRole role, unsigned char first_byte) {

    while (true) {
        if (state == READ_START_FLAG && first_byte == FLAG) {
            state = READ_ADDRESS;
            temp[i_pos++] = first_byte;
            return -1;
        } else if (state == READ_ADDRESS && first_byte != FLAG) {
            state = READ_CONTROL;
            temp[i_pos++] = first_byte;
            return -1;
        } else if (state == READ_CONTROL){
            temp[i_pos++] = first_byte;
            if (first_byte == FLAG) {
                state = READ_BCC;
            } else {
                return -1;
            }
        } else if (state == READ_BCC) {
            if ((temp[3] == (temp[1] ^ temp[2])) && (i_pos > 4)) {
                state = READ_END_FLAG;
            } else { 
                state = READ_START_FLAG; 
                i_pos = 0; 
                return -1;
            }
        } else if (state == READ_END_FLAG) {
            state = READ_START_FLAG;
            i_pos = 0;
            if (role == LlRx) {
                if (temp[2] == SET) {
                    temp[2] = 0x07;
                    temp[3] = (temp[1] ^ temp[2]);
                    temp[4] = 0x7E;
                    write(fd, temp, PACKET_SIZE);
                    state = 0;
                    i_pos = 0;
                    kill_alarm();
                    LOG_AL_EVENT("SUCCESS: in check_supervision_frame() - role: %d\n", role)
                    return 0;
                }
            } else if (role == LlTx) {
                if (temp[2] == UA) {
                    state = 0;
                    i_pos = 0;
                    kill_alarm();
                    LOG_AL_EVENT("SUCCESS: in check_supervision_frame() - role: %d\n", role)
                    return 0;
                }
            }
            return -1;
        } else {
            return -1;
        }
    }
}

int resState = 0;
unsigned char resSavedChars[PACKET_SIZE] = {};
int resptr = 0;

int check_i_frame(unsigned char val, int fd, int ca) {  
    while (TRUE) {
        switch (resState)
        {
        case 0:
            if (val == 0x7E) {
                resState = 1;
                resSavedChars[resptr++] = val;
                return 0;
            }
            break;   
        case 1:
            if (val != 0x7E) {
                resState = 2;
                resSavedChars[resptr++] = val;
                return 0;
            }
        case 2:
            resSavedChars[resptr++] = val;
            if (val == 0x7E){
                resState = 3;
            } else {
                return 0;
            }
        case 3:
            if (resSavedChars[3] == (resSavedChars[1] ^ resSavedChars[2]) && resptr > 4) {
                state = READ_END_FLAG;
            } else {
                resState = 0;
                resptr = 0;
                return 0;
            } 
        case 4:
            resState = 0;
            resptr = 0;
            if (resSavedChars[2] == RR(0)) {
                LOG_AL_EVENT("RR0 received. \n")
                resState = 0;
                resptr = 0;
                if (ca == 0) return 1; else return -1;
            } else if (resSavedChars[2] == RR(1)) {
                LOG_AL_EVENT("RR1 received. \n")
                resState = 0;
                resptr = 0;
                if (ca == 1) return 1; else return -1;
            } else if (resSavedChars[2] == REJ(0)) {
                LOG_AL_EVENT("REJ0 received. \n")
                resState = 0;
                resptr = 0;
                return -1;
            } else if (resSavedChars[2] == REJ(1)) {
                LOG_AL_EVENT("REJ1 received. \n")
                resState = 0;
                resptr = 0;
                return -1;
            }
            return -1;
        default:
            break;
        }
        return 0;
    }
}

int send_set(int fd) {
    unsigned char set[5] = {FLAG, SET_A_ER, SET, SET_A_ER ^ SET, FLAG};
    int bytes = write(fd, set, 5);
    LOG_AL_EVENT("SET flag sent, %d bytes written\n", bytes)
    return bytes;
}

int send_supervision_frame(int fd, int t, int ca) {
    unsigned char msg[5] = {FLAG, SET_A_ER, SET, SET_A_ER ^ SET, FLAG};

    if (t == 0) {
        if (ca == 0) 
            msg[2] = REJ(0);
        else
            msg[2] = REJ(1);
    } else {
        if (ca == 0) 
            msg[2] = RR(0);
        else
            msg[2] = RR(1);
    }
    msg[3] = (msg[1] ^ msg[2]);

    int bytes = write(fd, msg, 5);
    if (t == 0) {
        LOG_AL_EVENT("REJ%d flag sent, %d bytes written\n", ca, bytes)
    } else {
        LOG_AL_EVENT("RR%d flag sent, %d bytes written\n", ca, bytes)
    }
    return bytes;
}


int connect_to_receiver(const int fd, const int nTries, const int timeout) {
    LOG_AL_EVENT("FD/Tries/Timeout: %d, %d, %d\n", fd, nTries, timeout)
    int nt = nTries;
    while (TRUE) {

        if (!alarmEnabled) {
            if (nt == 0) {
                LOG_AL_ERROR("[emitter]: time out in connection_to_receiver\n")
                return TOO_MANY_ATTEMPTS;
            }
            send_set(fd);
            LOG_AL_EVENT("[emitter]: set sent %d\n", nt)
            nt--;
            setup_alarm(timeout);
        }

        unsigned char buf[PACKET_SIZE] = {0};
        int bytes = read(fd, buf, 1);
        if (check_supervision_frame(fd, LlTx, buf[0]) == 0) return 0; 
    }
    LOG_AL_ERROR("Unreachable situation\n")
}

int connect_to_emitter(int fd) {
    unsigned char buf[PACKET_SIZE] = {0};

    while (TRUE) {
        int bytes = read(fd, buf, 1);
            if (check_supervision_frame(fd, LlRx, buf[0]) == 0) 
                return 0;
 
        }
    return -1;
}

int emitter_send_disc(int fd) {
    unsigned char msg[5] = {FLAG, SET_A_ER, DISC, SET_A_ER ^ DISC, FLAG};

    int bytes = write(fd, msg, 5);
    LOG_AL_EVENT("[emitter]: DISC flag sent, %d bytes written\n", bytes)
    return bytes;
}

int await_disc(fd) {
    unsigned char buf[PACKET_SIZE] = {0};
    int bytes = read(fd, buf, 1);
    if (buf != 0 && bytes > -1) {
        int ans = closeState(buf[0], fd);
        if (ans == 2) {
            kill_alarm();
            return 1;
        }
    }

    return 0;
}

int disconnect_from_receiver(int nTries, int timeout, int fd) {
    int nt = nTries;

    while (true) {

        if (!alarmEnabled) {
            if (nt == 0) {
                return 0;
            }
            emitter_send_disc(fd);
            nt--;
            setup_alarm(timeout);
        }

        if (await_disc(fd) == 1)  {
            emitter_send_disc_ua(fd);
            return 1;
        }
    }
    return 0;
}

int endState = 0;
unsigned char endSavedChars[PACKET_SIZE] = {};
int end_pointer = 0;

int closeState(unsigned char byte, int fd) {
    while (true) {
        switch (endState)
        {
            case 0:
                if (byte == FLAG) {
                    endState = 1;
                    endSavedChars[end_pointer++] = byte;
                    return 0;
                }
                break;   
            case 1:
                if (byte == SET_A_RE) {
                    endState = 2;
                    endSavedChars[end_pointer++] = byte;
                    return 0;
                } else if (byte == SET_A_ER) {
                    endState = 2;
                    endSavedChars[end_pointer++] = byte;
                    return 0;
                }
                break;
            case 2:
                if (byte != FLAG) {
                    endState = 3;
                    endSavedChars[end_pointer++] = byte;
                    return 0;
                }
                break;
            case 3: 
                if (byte == (endSavedChars[2] ^ endSavedChars[1])) {
                    endState = 4;
                    endSavedChars[end_pointer++] = byte;
                    return 0;
                } else {
                    LOG_AL_ERROR("Procotol error.\n")
                    endState = 1;
                    end_pointer = 0;
                    return -1;
                }
                break;
            case 4:
                if (byte == 0x7E) {
                    endState = 0;
                    end_pointer = 0;
                    if (endSavedChars[2] == UA) {
                        return 3;
                    } else if (endSavedChars[2] == DISC && endSavedChars[1] == SET_A_RE) {
                        return 2;
                    } else if (endSavedChars[2] == DISC && endSavedChars[1] == SET_A_ER) {
                        return 1;

                    }
                    return 0;
                }
                break;
        }
    }
}

int receiver_await_disc(int fd) { 
    unsigned char buf[PACKET_SIZE] = {0};

    int bytes = read(fd, buf, 1);

    if (buf != 0 && bytes > -1) {
        int ans = closeState(buf[0], fd);
        if (ans == 1) {
            return 1;
        }
    }

    return 0;
}


int emitter_send_disc_ua(int fd) {
    unsigned char msg[5] = {FLAG, SET_A_RE, UA, SET_A_RE ^ UA, FLAG};

    int bytes = write(fd, msg, 5);
    LOG_AL_EVENT("[emitter]: UA flag sent, %d bytes written\n", bytes)
    return bytes;
}


int receiver_send_disc(fd) {
    unsigned char msg[5] = {FLAG, SET_A_RE, DISC, SET_A_RE ^ DISC, FLAG};

    int bytes = write(fd, msg, 5);
    LOG_AL_EVENT("[receiver]: DISC flag sent, %d bytes written\n", bytes)
    return bytes;
}

int receiver_await_ua(int fd) {
    unsigned char buf[PACKET_SIZE] = {0};

    int bytes = read(fd, buf, 1);

    if (buf != 0 && bytes > -1) {
        int ans = closeState(buf[0], fd);
        if (ans == 3) {
            return 1;
        }
    }
    return 0;
}

int nTrans = 0;

int disconnect_from_emitter(int nTries, int timeout, int fd) {
    while(true) {
        int ans = receiver_await_disc(fd);
        if (ans) {
            break;
        }
    }

    nTrans = nTries;
    while (true) {

        if (!alarmEnabled) {
            if (nTrans == 0) {
                return -1;
            }
            receiver_send_disc(fd);
            nTrans--;
            setup_alarm(timeout);
        }

        if (receiver_await_ua(fd) == 1)  {
            emitter_send_disc_ua(fd);
            return 0;
        }
    }
    return -1;
}

int send_i_frame(unsigned char frame[], int frameSize, int fd) {
    int bytes = write(fd, frame, frameSize);
    return bytes;
}
