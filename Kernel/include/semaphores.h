#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>

#define MAX_SEMAPHORES 4096

typedef uint16_t sem_t;

typedef struct SemaphoreManagerCDT *SemaphoreManagerADT;

void semaphore_manager_init();

int8_t sem_init(sem_t *sem, uint32_t initialValue);
int8_t sem_open(sem_t *sem);
int8_t sem_close(sem_t *sem);
int8_t sem_destroy(sem_t *sem);
int8_t sem_post(sem_t *sem);
int8_t sem_wait(sem_t *sem);

#endif
