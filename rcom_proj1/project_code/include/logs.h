#ifndef _LOGS_H_
#define _LOGS_H_

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

FILE *al_events;
FILE *al_errors;
FILE *ll_events;
FILE *ll_errors;
FILE *driver_events;
FILE *driver_errors;

#define LOG_AL_EVENT(...) \
    if (al_events != NULL) { \
        fprintf(al_events, "%ld;", time(0)); fprintf(al_events, __VA_ARGS__); \
        }
#define LOG_AL_ERROR(...) \
    if (al_errors != NULL) { \
        fprintf(al_errors, "%ld;", time(0)); fprintf(al_errors, __VA_ARGS__); \
        }
#define LOG_LL_EVENT(...) \
    if (ll_events != NULL) { \
        fprintf(ll_events, "%ld;", time(0)); fprintf(ll_events, __VA_ARGS__); \
        }
#define LOG_LL_ERROR(...) \
    if (ll_errors != NULL) { \
        fprintf(ll_errors, "%ld;", time(0)); fprintf(ll_errors,__VA_ARGS__); \
        }
#define LOG_DRIVER_EVENT(...) \
    if (driver_events != NULL) { \
        fprintf(driver_events, "%ld;", time(0)); fprintf(driver_events,__VA_ARGS__); \
        }
#define LOG_DRIVER_ERROR(...) \
    if (driver_errors != NULL) { \
        fprintf(driver_errors, "%ld;", time(0)); fprintf(driver_errors,__VA_ARGS__); \
        }

int init_al_logs();

int init_ll_logs();

int init_driver_logs();
/*
void LOG_AL_EVENT(char *stri);
void LOG_AL_ERROR(char *stri);

void LOG_LL_EVENT(char *stri);
void LOG_LL_ERROR(char *stri);

void LOG_DRIVER_EVENT(char *stri);
void LOG_DRIVER_ERROR(char *stri);
*/

#endif // _LOGS_H_