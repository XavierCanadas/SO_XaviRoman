#include "crc.h"
#include "fileManager.h"
#include "myutils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep function : waits for seconds
//#include "timer.h"

FileManager fm;
int errors = 0;

pthread_mutex_t lock;


void *worker_function(void *arg) {
    int nBytesReadData = 1;

    while (fm.nFilesRemaining > 0) {

        // Passa un thread perquè reservi un fitxer.
        //my_sem_wait(&semafor);

        dataEntry d;
        char buff[256];
        crc codiComputatCRC;

        // El thread q ha passat executa la funció per agafar un fitxer q llegirà. Els locks es fan dintre de la funció.
        int res = getAndReserveFile(&fm, &d);

        if (res == 0) {
            // es llegeix el fitxer
            read(d.fdcrc, &codiComputatCRC, sizeof(crc));
            int nBytesReadData = read(d.fddata, buff, 255);
	        //printf("%d - %s\n", nBytesReadData, buff );
            crc crcLlegit = crcSlow((unsigned char* const) buff, nBytesReadData);

            // es fa lock perquè es toca una variable comuna;
            pthread_mutex_lock(&lock);
            if (fm.fileFinished[d.index] == 0 && codiComputatCRC != crcLlegit) {
                errors++;
            }

            pthread_mutex_unlock(&lock);


            // Després de llegir i fer les operacions del crc, es crida a unreserveFile posar-la disponible per llegir un altre bloc.
            unreserveFile(&fm, &d);



            // Si s'ha arribat al final, es marca com acabada.
            if (nBytesReadData <= 0) {
            	
                markFileAsFinished(&fm, &d);
            }
        }

    }
    return NULL;
}

int main(int argc, char **argv) {

    startTimer(0);
    initialiseFdProvider(&fm, argc, argv);

    pthread_mutex_init(&lock, NULL);

    int N = 4;
    pthread_t threadID[N];
    for (int i = 0; i < N; ++i) {
        pthread_t thread;
        pthread_create(&threadID[i], NULL, worker_function, NULL);
    }


    for (int i = 0; i < N; ++i) {
        pthread_join(threadID[i], NULL);
    }

    printf("\n**********************************************************************************************\n");
    printf("\nS'han processat %d fitxers i hi ha hagut %d errors!\n", argc - 1, errors);

    pthread_mutex_destroy(&lock);

    destroyFdProvider(&fm);

    printf("El temps que s'ha trigat en verificar els fitxers és: %ld\n\n", endTimer(0));
    printf("\n**********************************************************************************************\n");


    exit(0);
}
