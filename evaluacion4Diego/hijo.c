#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_SIZE 1024
#define SHM_NAME "/shared_memory"
#define MUTEX_NAME "/shared_mutex"

int main() {
    // Abrir la región de memoria compartida existente
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Mapear la memoria compartida en la dirección de memoria del hijo
    int* shared_memory = (int*)mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // Abrir el mutex compartido
    sem_t* mutex = sem_open(MUTEX_NAME, 0);

    FILE* fp = fopen("resultados.txt", "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    int values[10];
    int i;

    for (i = 0; i < 10; i++) {
        // Bloquear el mutex
        sem_wait(mutex);

        int num = *shared_memory;
        printf("Hijo: leyendo %d\n", num);
        values[i] = num;

        // Desbloquear el mutex
        sem_post(mutex);

        sleep(1); // Esperar 1 segundo
    }

    // Calcular el promedio
    int sum = 0;
    for (i = 0; i < 10; i++) {
        sum += values[i];
    }
    float average = (float)sum / 10;

    // Escribir los valores y el promedio en el archivo
    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f\n", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], average);
    fclose(fp);

    // Liberar recursos
    munmap(shared_memory, SHM_SIZE);
    sem_close(mutex);

    return 0;
}
