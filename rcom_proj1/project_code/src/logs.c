#include "../include/logs.h"

int init_al_logs() {
    al_events = fopen("al_events.log", "a");
    al_errors = fopen("al_errors.log", "a");

    if (al_errors == NULL || al_events == NULL) return -1;

    return 0;
}

int init_ll_logs() {
    ll_events = fopen("ll_events.log", "a");
    ll_errors = fopen("ll_errors.log", "a");

    if (ll_events == NULL || ll_errors == NULL) return -1;

    return 0;
}

int init_driver_logs() {
    driver_events = fopen("driver_events.log", "a");
    driver_errors = fopen("driver_errors.log", "a");

    if (driver_events == NULL || driver_errors == NULL) return -1;

    return 0;
}
