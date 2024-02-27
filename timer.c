#include "timer.h"

struct timeval start[1];
void startTimer() {
    int i =0;
    if(i>=100) printf("Warning only 100 timers available!!");
    gettimeofday(&start[i], NULL);
}

long endTimer() {
    int i = 0;
    long mtime, seconds, useconds;    
    struct timeval end;

    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start[i].tv_sec;
    useconds = end.tv_usec - start[i].tv_usec;
    mtime = ((seconds) * 1000000 + useconds) + 0.5;
    return mtime;
}