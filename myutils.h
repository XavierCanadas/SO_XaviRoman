#ifndef MYUTILS
#define MYUTILS

#include <sys/time.h>
#include <sys/types.h> 
#include <netdb.h> 
#include <unistd.h>     // unix-like system calls read and write
#include <fcntl.h>      // unix-like file handling : open
#include <stdlib.h>     // standard C lib input output basic functions compatible with Windows
#include <stdio.h>      // standard C lib input output basic functions compatible with Windows
#include <stdbool.h>
#include <string.h>     // also from standard C lib : basic string functions like strlen
#include <pthread.h>


void startTimer(int i);   // start timer i
long endTimer(int i);     // returns millis since timer i started

int get_file_size(char* fname);

int read_split( int fin, char* buff, int maxlen, char* ch_end );

// Semaphores implementation Using pthreads monitors
typedef struct semaphore_struct {
	int i;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} my_semaphore;

typedef struct {
  bool bInUse;
  pthread_mutex_t lock;
  pthread_cond_t cond_free;
  
} monitor_lock;

void my_sem_init(my_semaphore* sem, int i);
void my_sem_wait(my_semaphore* sem);
void my_sem_signal(my_semaphore* sem);
#endif