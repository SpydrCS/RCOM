#ifndef RCOM_PROJ2_FTP_INCLUDE_CLIENTTCP_H
#define RCOM_PROJ2_FTP_INCLUDE_CLIENTTCP_H

#include "../include/parse.h"

int getLastStatus(char *buf);
int getFilename(char *buf, char* filename);
int getPortNumber(char* buf);
int newSocket(char *ip, int port);
int setConnection(char* ip, int port, struct parse_info *info);

#endif // RCOM_PROJ2_FTP_INCLUDE_CLIENTTCP_H
