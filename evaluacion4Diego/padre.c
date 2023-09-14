#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_SIZE 1024
#define SHM_NAME "/shared_memory"
#define MUTEX_NAME "/shared_mutex"

int main() {
    srand(time(NULL));

    // Crear una región de memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);

    // Mapear la memoria compartida en la dirección de memoria del padre
    int* shared_memory = (int*)mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Crear un mutex compartido
    sem_t* mutex = sem_open(MUTEX_NAME, O_CREAT, 0666, 1); // Inicialmente, el mutex está desbloqueado

    FILE* fp = fopen("resultados.txt", "w");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    int i;
    for (i = 0; i < 10; i++) {
        sleep(2); // Esperar 2 segundos
        int num = rand() % 10 + 1;

        // Bloquear el mutex
        sem_wait(mutex);

        // Escribir el número en memoria compartida
        *shared_memory = num;
        printf("Padre: escribiendo %d\n", num);

        // Desbloquear el mutex
        sem_post(mutex);
    }

    fclose(fp);

    // Esperar a que el hijo termine
    wait(NULL);

    // Leer y mostrar el contenido de la memoria compartida
    printf("Contenido de la memoria compartida:\n");
    for (i = 0; i < 10; i++) {
        // Bloquear el mutex
        sem_wait(mutex);

        int num = *shared_memory;
        printf("%d ", num);
        fprintf(fp, "%d,", num);
        fflush(fp);

        // Desbloquear el mutex
        sem_post(mutex);
    }
    fprintf(fp, "\n");
    fclose(fp);

    // Liberar recursos
    munmap(shared_memory, SHM_SIZE);
    shm_unlink(SHM_NAME);
    sem_close(mutex);
    sem_unlink(MUTEX_NAME);

    return 0;
}
