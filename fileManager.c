#include "fileManager.h"


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


    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        char path[100];
        strcpy(path, argv[i + 1]);
        strcat(path, ".crc");
        fm->fdData[i] = open(argv[i + 1], O_RDONLY);
        fm->fdCRC[i] = open(path, O_RDONLY);

        fm->fileFinished[i] = 0;
        fm->fileAvailable[i] = 1;

        printf("fitxer %s, amb codi %d\n", argv[i + 1], fm->fdData[i]);
        printf("fitxer crc %s, amb codi %d\n", path, fm->fdCRC[i]);


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
    // This function needs to be implemented by the students
    int i;

    // Fer lock perquè el thread comprovi si hi ha algun fitxer disponible.
    pthread_mutex_lock(&lock);

    for (i = 0; i < fm->nFilesTotal; ++i) {
        if (fm->fileAvailable[i] && !fm->fileFinished[i]) {

            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;

            // You should mark that the file is not available
            *fm->fileAvailable = 0;

            // Una vegada s'ha trobat el fitxer i assignat al dataEntry, es fa un unlock.
            pthread_mutex_unlock(&lock);

            return 0;
        }
    }
    // Si no s'ha trobat cap fitxer es fa unlock.
    pthread_mutex_unlock(&lock);

    return 1;
}

void unreserveFile(FileManager *fm, dataEntry *d) {
    // es fa lock perquè es tocarà memòria compartida
    pthread_mutex_lock(&lock);

    fm->fileAvailable[d->index] = 1;

    my_sem_signal(&semafor);

    pthread_mutex_unlock(&lock);
}

void markFileAsFinished(FileManager *fm, dataEntry *d) {
    // es fa lock perquè es tocarà memòria compartida
    pthread_mutex_lock(&lock);

    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; //mark that a file has finished



    if (fm->nFilesRemaining == 0) {
        printf("\nAll files have been processed\n");
        //TO COMPLETE: unblock all waiting threads, if needed

        // el signal el fem al unreverseFile i aquest sempre s'executa abans de cridar a aquesta funció
    }

    pthread_mutex_unlock(&lock);
}