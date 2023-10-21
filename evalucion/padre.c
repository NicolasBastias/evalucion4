#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define SHM_SIZE 1024
#define SHM_KEY 12345
#define SEM_KEY 54321

int main() {
    srand(time(NULL));

    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }
    int* shared_memory = shmat(shmid, NULL, 0);

    int semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    FILE* fp = fopen("resultados.txt", "w");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    int i;
    for (i = 0; i < 10; i++) {
        sleep(2);
        int num = rand() % 10 + 1;

        struct sembuf sem_lock = {1, -1, SEM_UNDO};
        struct sembuf sem_unlock = {0, 1, SEM_UNDO};

        semop(semid, &sem_lock, 1);

        *shared_memory = num;
        printf("Padre: escribiendo %d\n", num);

        semop(semid, &sem_unlock, 1);
    }

    fclose(fp);

    shmdt(shared_memory);

    return 0;
}