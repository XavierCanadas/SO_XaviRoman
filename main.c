#include "crc.h"
#include "fileManager.h"
#include "myutils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep function : waits for seconds

FileManager fm;
int errors = 0;


void *worker_function(void *arg) {
    int nBytesReadData = 1;

    while (fm.nFilesRemaining > 0) {

        // Passa un thread perquè reservi un fitxer.
        my_sem_wait(&semafor);

        dataEntry d;
        char buff[256];
        crc codiComputatCRC;

        // El thread q ha passat executa la funció per agafar un fitxer q llegirà. Els locks es fan dintre de la funció.
        int res = getAndReserveFile(&fm, &d);

        my_sem_signal(&semafor);
        pthread_mutex_unlock(&lock);


        if (res == 0) {
            //pthread_mutex_lock(&lock);


            // es llegeix el fitxer
            read(d.fdcrc, &codiComputatCRC, sizeof(crc));
            nBytesReadData = read(d.fddata, buff, 255);

            crc crcLlegit = crcSlow((unsigned char* const) buff, nBytesReadData);

            if (codiComputatCRC != crcLlegit ) {
                errors++;
                printf("\nCRC error in file %d\n", d.fddata);
                printf("el crc ha donat %d i el fitxer hi havia %d\n", codiComputatCRC, crcLlegit);
            }
            //pthread_mutex_unlock(&lock);


            // Després de llegir i fer les operacions del crc, es crida a unreserveFile posar-la disponible per llegir un altre bloc.
            unreserveFile(&fm, &d);



            // Si s'ha arribat al final, es marca com acabada.
            if (nBytesReadData <= 0) {
                markFileAsFinished(&fm, &d);
            }
        }

    }
    my_sem_signal(&semafor);

    return NULL;
}

int main(int argc, char **argv) {


    initialiseFdProvider(&fm, argc, argv);

    pthread_mutex_init(&lock, NULL);

    // S'inicialitza el semàfor. L'espai q es dona és del nombre de fitxers que s'ha passat.
    my_sem_init(&semafor, 1);

    int N = 10;

    pthread_t threadID[N];
    for (int i = 0; i < N; ++i) {
        pthread_t thread;
        pthread_create(&threadID[i], NULL, worker_function, NULL);
    }


    for (int i = 0; i < N; ++i) {
        pthread_join(threadID[i], NULL);
    }

    printf("\nHi ha hagut %d errors\n\n", errors);

    //pthread_mutex_destroy(&lock);

    destroyFdProvider(&fm);


    /* Comprovació del fitxer sense threads
    int file, fileCRC, errors = 0;

    char path[100];
    strcpy(path, argv[1]);
    strcat(path, ".crc");
    file = open(argv[1], O_RDONLY);
    fileCRC = open(path, O_RDONLY);

    char buffer[256];
    crc crcComputat, crcLlegit;

    while (read(file, buffer, 255) > 0) {
        crcComputat = crcSlow((unsigned char* const) buffer, 255);

        read(fileCRC, &crcLlegit, sizeof(crc));

        if (crcLlegit != crcComputat)
            errors++;
    }
    printf("Hi ha hagut %d errors\n", errors);
     */


    exit(0);
}