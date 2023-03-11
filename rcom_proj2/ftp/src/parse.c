#include "../include/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printParseInfo(const struct parse_info *info) {
    printf("protocol: \t%s\n", info->protocol);
    printf("username: \t%s\n", info->username);
    printf("password: \t%s\n", info->password);
    printf("hostname: \t%s\n", info->hostname);
    printf("path: \t%s\n", info->pathURL);
}

int getInfo(char *url, struct parse_info *info) {
    memset(info, 0, sizeof(*info));
    parseProtocol(url, info, &url);
    parseAccount(url, info, &url);
    parseHostname(url, info, &url);
    parsePath(url, info);
    //printParseInfo(info);
    return 0;
}

int parseProtocol(char *url, struct parse_info *info, char **rest) {
    // Look for string of the form <protocol>://
    char *s = strsep(&url, ":");

    // If the prefix is not found, or we are missing "//", we can't continue.
    if (!url || !strlen(s) || strncmp("//", url, 2) != 0) {
        return -1;
    }

    info->protocol = s;

    // Skip the possible repeated "/" characters
    while (*url == '/') ++url;
    *rest = url;

    return 0;
}

int parseAccount(char *url, struct parse_info *info, char **rest) {
    // Look for a string of the form "username:"
    char *temp = url;
    char *s = strsep(&url, ":");

    // If the string is not found, this field is ignored.
    // However, no password can exist either.
    if (!url || !strlen(s)) {
        //printf("REST: %s\n", url);
        url = temp;
        info->username = "anonymous";
        info->password = "qualquer-password";
    } else {
        info->username = s;
        parsePassword(url, info, &url);
    }
    *rest = url;
    return 0;
}

int parsePassword(char *url, struct parse_info *info, char **rest) {
    // If we have a username, we need a password.
    // Look for a string of the form "password@"
    char *s = strsep(&url, "@");

    // If the string is not found, we can't continue.
    if (!url || !strlen(s)) {
        fprintf(stdout, "A username was specified without a password.\n");
        exit(-1);
    }
    info->password = s;
    *rest = url;
    return 0;
}

int parseHostname(char *url, struct parse_info *info, char **rest) {
    // Look for a string of the form "hostname/"
    char *s = strsep(&url, "/");

    // If the string is not found, we can't continue.
    if (!url || !strlen(s)) {
        fprintf(stdout, "A hostname has not been specified.\n");
        return -1;
    } else {
        info->hostname = s;
    }
    *rest = url;
    return 0;
}

int parsePath(const char *url, struct parse_info *info) {
    // The remainder of the string is the path.
    if (strlen(url)) {
        info->pathURL = url;
    } else {
        fprintf(stdout, "A path has not been specified.\n");
        return -1;
    }
    return 0;
}
