#ifndef _ALARM_H_
#define _ALARM_H_

#include "logs.h"
#include "link_layer.h"

extern int alarmCount;
extern int alarmEnabled;

int setup_alarm(unsigned int duration);
void sigalrm_handler(int signal);
void kill_alarm();

#endif
