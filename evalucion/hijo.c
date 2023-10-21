#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SHM_SIZE 1024
#define SHM_KEY 12345
#define SEM_KEY 54321

int main() {
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

    FILE* fp = fopen("resultados.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    int values[10];
    int i;

    for (i = 0; i < 10; i++) {
        struct sembuf sem_lock = {0, -1, SEM_UNDO};
        struct sembuf sem_unlock = {1, 1, SEM_UNDO};

        semop(semid, &sem_lock, 1);

        int num = *shared_memory;
        printf("Hijo: leyendo %d\n", num);
        values[i] = num;

        semop(semid, &sem_unlock, 1);

        sleep(1);
    }

    // Calcular el promedio
    int sum = 0;
    for (i = 0; i < 10; i++) {
        sum += values[i];
    }
    float average = (float)sum / 10;

    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f\n", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], average);
    fclose(fp);

    shmdt(shared_memory);

    return 0;
}
