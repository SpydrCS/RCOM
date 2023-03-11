#ifndef RCOM_PROJ2_FTP_INCLUDE_PARSE_H
#define RCOM_PROJ2_FTP_INCLUDE_PARSE_H

struct parse_info {
    const char *protocol;
    const char *username;
    const char *password;
    const char *hostname;
    const char *pathURL;
};

void printParseInfo(const struct parse_info *info);
int getInfo(char *url, struct parse_info *info);
int parseProtocol(char *url, struct parse_info *info, char **rest);
int parseAccount(char *url, struct parse_info *info, char **rest);
int parsePassword(char *url, struct parse_info *info, char **rest);
int parseHostname(char *url, struct parse_info *info, char **rest);
int parsePath(const char *url, struct parse_info *info);

#endif // RCOM_PROJ2_FTP_INCLUDE_PARSE_H
