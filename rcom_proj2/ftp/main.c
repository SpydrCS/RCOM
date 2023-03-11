#include <stdio.h>
#include <stdlib.h>
#include "include/parse.h"
#include "include/getip.h"
#include "include/clientTCP.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL of resource to be downloaded>\n"
            "URL must be like: ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    }

    struct parse_info parseInfo;

    if (getInfo(argv[1], &parseInfo) < 0) {
        return -1;
    }

    char ipAddress[15];

    if(getIPAddress(parseInfo.hostname, ipAddress) < 0) {
        return -1;
    }

    if (setConnection(ipAddress, 21, &parseInfo) < 0) {
        return -1;
    }

    return 0;
}