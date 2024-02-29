#include "fileManager.h"

my_semaphore semafor;

/*
 * Inicialitzar mutex
 * Inicialitzar nº arxius
 * totals q resten per processar
 * Assignar memòria pels fd i altres coses.
 */
void initialiseFdProvider(FileManager *fm, int argc, char **argv) {

    // Complete the initialisation
    /* Your rest of the initailisation comes here*/
    fm->nFilesTotal = argc - 1;
    fm->nFilesRemaining = fm->nFilesTotal;
    // Initialise enough memory to  store the arrays
    fm->fdData = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fdCRC = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileFinished = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileAvailable = malloc(sizeof(int) * fm->nFilesTotal);

    my_sem_init(&semafor, 1);


    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        char path[100];
        strcpy(path, argv[i + 1]);
        strcat(path, ".crc");
        fm->fdData[i] = open(argv[i + 1], O_RDONLY);
        fm->fdCRC[i] = open(path, O_RDONLY);

        fm->fileFinished[i] = 0;
        fm->fileAvailable[i] = 1;

        if(fm->fdData[i] < 0){
            printf("could not open file");
        }

        if(fm->fdCRC[i] < 0){
            printf("could not open crc file");
        }
    }
}

/*
 * Tancar cada fd
 * Alliberar memòria
 */
void destroyFdProvider(FileManager *fm) {
    int i;
    for (i = 0; i < fm->nFilesTotal; i++) {
        close(fm->fdData[i]);
        close(fm->fdCRC[i]);
    }
    free(fm->fdData);
    free(fm->fdCRC);
    free(fm->fileFinished);
}

// S'ha afegit un lock
int getAndReserveFile(FileManager *fm, dataEntry *d) {

    for (int i = 0; i < fm->nFilesTotal; ++i) {
        my_sem_wait(&semafor);
        if (fm->fileAvailable[i] == 1 && fm->fileFinished[i] == 0) {
            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;

            // iniciar el cronòmetre per al fitxer actual
            startTimer(i);

            // You should mark that the file is not available
            fm->fileAvailable[i] = 0;

            // Una vegada s'ha trobat el fitxer i assignat al dataEntry, es fa un signal.
            my_sem_signal(&semafor);


            return 0;
        }
        my_sem_signal(&semafor);
    }
    return 1;
}

void unreserveFile(FileManager *fm, dataEntry *d) {
    fm->fileAvailable[d->index] = 1;
    endTimer(d->index);

}

void markFileAsFinished(FileManager *fm, dataEntry *d) {
    // es fa lock perquè es tocarà memòria compartida
    my_sem_wait(&semafor);

    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; //mark that a file has finished
    my_sem_signal(&semafor);
    if (fm->nFilesRemaining == 0) {
        printf("\nAll files have been processed\n");
    }
}
