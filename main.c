#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>   // for usleep waits for microsec
#include <unistd.h> // for sleep: waits for seconds
#include "crc.h"
#include "fileLock.h"
#define N 4


typedef struct {
    int nBlock;
    int isGet;
} Request;

typedef struct {
    int nBlock;
    short int crc;
} Result;

pthread_mutex_t lock;

int main(int argc, char * argv[]) {
    int pipeA[2], pipeB[2];
    // Create two pipes. Also remember to close the channels when needed, otherwise it will not work!
    pipe(pipeA);
    pipe(pipeB);
    pthread_mutex_init(&lock,NULL);
    for (int i = 0; i < N; ++i) {
        int n = fork();
        if (n == 0) {
            close(pipeB[0]);
            close(pipeA[1]);
            int fd = open(argv[1], O_RDONLY);
            int fdCRC =open(argv[2], O_RDWR);
            Request r;
            char buff[256];
            //mentres llegim per la pipe
            while( read(pipeA[0],&r,sizeof(Request))>0 /* Read a request from the pipe */) {
                if (!r.isGet) {
                //generate
                    crc computed;
                    //pthread_mutex_lock(&lock);
                    file_lock_write(fd, r.nBlock*256,sizeof(char)*256/*buff array*/);
                    lseek(fd,r.nBlock*256,SEEK_SET);
                    read(fd,buff,sizeof(char)*256);

                    //pthread_mutex_unlock(&lock);
                    file_unlock(fd, r.nBlock*256,sizeof(char)*256/*buff array*/);
                    close(fd);
                    computed = crcSlow((unsigned char* const) buff, 256);

                   

                    file_lock_read(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));
                    file_lock_write(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));

                    lseek(fdCRC,r.nBlock*sizeof(crc),SEEK_SET);
                    write(fdCRC,&computed,sizeof(crc));
                    
                    file_unlock(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));
                    

                    /* Recompute the CRC, use lseek to get the correct datablock,
                    and store it in the correct position of the CRC file. Remember to use approppriate locks! */
                    usleep(rand()%1000 *1000); // Make the computation a bit slower
                    
                }
                else{
                //get
                    usleep(rand()%1000 *1000);
                    Result res;
                    res.nBlock = r.nBlock;
                    // Read the CRC from the CRC file, using lseek + read. Remember to use the correct locks!
                    
                    file_lock_read(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));
                    file_lock_write(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));

                    lseek(fdCRC,r.nBlock*sizeof(crc),SEEK_SET);
                    read(fdCRC,&res.crc,sizeof(crc));

                    file_unlock(fdCRC,r.nBlock*sizeof(crc),sizeof(crc));


                    write(pipeB[1],&res,sizeof(Result));
                    
                }

            }
            close(pipeA[0]);
            close(pipeB[1]);
            close(fd);
            close(fdCRC);

            exit(0);
        }
    }
    close(pipeA[0]);
    close(pipeB[1]);
    char s[100];
    int nBytesRead;
    /* Read until the standard output*/
    while((nBytesRead = read(0, s, 100) ) > 0) {
        char op[200];
        s[nBytesRead] = '\0';
        int nBlock;

        sscanf(s, "%s %d", op, &nBlock);
        Request r;
        r.nBlock = nBlock;
        r.isGet = strcmp(op, "get") == 0;
        // Write r in the pipe!
//falten els close
        write(pipeA[1], &r,sizeof(Request));
    }
    close(pipeA[1]);

    printf("FINISHED\n");
    while(wait(NULL) == -1);

    // Now that is finished, write all the results
    Result res;

    while((nBytesRead = read(pipeB[0], &res, sizeof(res)) ) > 0) {
        printf("The CRC of block #%d is %u \n", res.nBlock, res.crc);
    }
    close(pipeB[0]);
    exit(0);
}