#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "../include/alarm.h"

int alarmEnabled = FALSE;
int alarmCount = 0;

int setup_alarm(unsigned int duration)
{
    (void)signal(SIGALRM, sigalrm_handler);

    if (alarmEnabled == FALSE)
    {
        alarm(duration);
        alarmEnabled = TRUE;
    }

    return 0;
}


void sigalrm_handler(int signal) {
    alarmEnabled = FALSE;
    alarmCount++;
    printf("Alarm #%d\n", alarmCount);
}


void kill_alarm() {
    alarmEnabled = FALSE;
    alarmCount = 0;
}