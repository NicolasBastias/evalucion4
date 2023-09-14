#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#define SHM_SIZE 1024
#define SHM_NAME "/shared_memory"
#define WRITE_MUTEX_NAME "/write_mutex"
#define READ_MUTEX_NAME "/read_mutex"

int main() {
    srand(time(NULL));

    // Abrir la región de memoria compartida existente
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);

    // Mapear la memoria compartida en la dirección de memoria del padre
    int* shared_memory = (int*)mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Abrir los mutex compartidos
    sem_t* write_mutex = sem_open(WRITE_MUTEX_NAME, O_CREAT, 0666, 1);
    sem_t* read_mutex = sem_open(READ_MUTEX_NAME, O_CREAT, 0666, 0);

    FILE* fp = fopen("resultados.txt", "w");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    int i;
    for (i = 0; i < 10; i++) {
        sleep(2); // Esperar 2 segundos
        int num = rand() % 10 + 1;

        // Bloquear el mutex de escritura
        sem_wait(write_mutex);

        // Escribir el número en memoria compartida
        *shared_memory = num;
        printf("Padre: escribiendo %d\n", num);

        // Desbloquear el mutex de lectura
        sem_post(read_mutex);

        // Desbloquear el mutex de escritura
        sem_post(write_mutex);
    }

    fclose(fp);

    // Liberar recursos
    munmap(shared_memory, SHM_SIZE);
    shm_unlink(SHM_NAME);
    sem_close(write_mutex);
    sem_unlink(WRITE_MUTEX_NAME);
    sem_close(read_mutex);
    sem_unlink(READ_MUTEX_NAME);

    return 0;
}
