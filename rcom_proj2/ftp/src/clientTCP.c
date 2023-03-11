/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include "../include/clientTCP.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

int getLastStatus(char *buf){
    int a;
    char *pnter;
    pnter = strtok (buf,"\n");
    while (pnter != NULL) {
        a = 0;
        a = atoi(pnter);
        pnter = strtok (NULL, "\n");
    }
    return a;
}

int getFilename(char *buf, char* filename){
    char *pnter;
    pnter = strtok (buf,"/");

    while (pnter != NULL) {
        strcpy(filename, pnter);
        pnter = strtok (NULL, "/");
    }

    return 0;
}

int getPortNumber(char* buf){
    int numb[5] = {0};
    int i = 0;
    char *pnter;
    pnter = strtok (buf,",");
    pnter = strtok (NULL,",");
    while (pnter != NULL) {
        int a = atoi(pnter);
        numb[i] = a;
        pnter = strtok (NULL, ",");
        i++;
    }
    return (numb[3]*256 + numb[4]);
}


int newSocket(char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr));

    return sockfd;
}

int setConnection(char* ip, int port_server, struct parse_info *info) {

    FILE* fileptr;

    int STOP = 0, visited = 0, sizeUsername = strlen(info->username), sizePassword = strlen(info->password), port = 0, download = 0, sizePath = strlen(info->pathURL);

    /* criaçao da string "user anonymous\r\n" "pass qualquer-password\r\n" a funcionar como desejado*/
    char usernameLogin[sizeUsername+7], passwordLogin[sizePassword+7], pathRecover[sizePath+7], filename[strlen(info->pathURL)];
    usernameLogin[0] = '\0';
    passwordLogin[0] = '\0';
    pathRecover[0] = '\0';

    strcat(usernameLogin, "user ");
    strcat(passwordLogin, "pass ");
    strcat(pathRecover, "retr ");

    strcat(usernameLogin, info->username);
    strcat(passwordLogin, info->password);
    strcat(pathRecover, info->pathURL);

    strcat(usernameLogin, "\r\n");
    strcat(passwordLogin, "\r\n");
    strcat(pathRecover, "\r\n");

    getFilename(info->pathURL, filename);

    char buf[500] = {0}, buf2[500]={0};

    size_t bytes, bytes2;

    int sockfd = newSocket(ip, port_server), sockfd2 = 0;
    if(sockfd == -1) return -1;

    while (!STOP)
    {  
        
        memset(buf, 0, 500);
        bytes = read(sockfd, buf, 500);
        
        if(download){
            memset(buf2, 0, 500);
            bytes2 = read(sockfd2, buf2, 500);
            if(bytes2 != -1 && bytes2 != 0) {
                printf("\nbuf2:");
                for(int i=0; i<bytes2; i++){
                    printf("%c", buf2[i]);
                    fputc(buf2[i], fileptr);
                }
            }
        }

        if(bytes == -1 || bytes == 0) {/* printf("\ni got nothing -- buf\n"); */ continue;}
        printf("\n%s\n", buf);
        int sc = getLastStatus(buf);

        //sending user
        if (sc == 220) {
            if(visited) continue;
            visited = 1;
            printf("\n---------------Sending user---------------\n");
            write(sockfd, usernameLogin, strlen(usernameLogin));
        //sending password
        } else if (sc == 331) {
            printf("\n---------------Sending password---------------\n");
            write(sockfd, passwordLogin, strlen(passwordLogin));
        //Log in succeeded
        } else if (sc == 230) {
            printf("\n---------------Entering passive mode---------------\n");
            write(sockfd, "pasv\r\n", 6);
        //entering passive mode
        } else if (sc == 227) {
            printf("\n---------------Calculating port number---------------\n");
            port = getPortNumber(buf);
            printf("\nPort Calculated: %d\n", port);

            if ((sockfd2 = newSocket(ip, port)) == -1) return -1;

            printf("\n---------------Retrieving file---------------\n");
            write(sockfd, pathRecover, strlen(pathRecover));
        //starting transfer
        } else if (sc == 150) {
            printf("\n---------------Starting Trnasfer---------------\n");
            fileptr = fopen(filename, "w");
            printf("\n--- Created file with name '%s' ---\n", filename);
            download = 1;
        //transfer complete
        } else if (sc == 226) {
            while(1){
                memset(buf2, 0, 500);
                bytes2 = read(sockfd2, buf2, 500);
                if(bytes2 != -1 && bytes2 != 0) {
                    printf("\nbuf2:");
                    for(int i=0; i<bytes2; i++){
                        printf("%c", buf2[i]);
                        fputc(buf2[i], fileptr);
                    }
                    printf("\n");
                }
                else{break;}
            }
            printf("\n---------------Transfer complete---------------\n");
            download = 0;
            STOP = 1;
        } else {
            printf("\n---------------Error: Status code %d unknown---------------\n", sc);
            return -1;
            break;
        }
    }

    fclose(fileptr);
    
    if (close(sockfd2)<0) {
        perror("close()");
        return -1;
    }

    if (close(sockfd)<0) {
        perror("close()");
        return -1;
    }
    return 0;
}
