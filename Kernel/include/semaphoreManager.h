#ifndef SEMAPHORE_MANAGER_H
#define SEMAPHORE_MANAGER_H

#include <stdint.h>

// Semaphore type
typedef uint16_t sem_t;

// Opaque type for semaphore manager
typedef struct SemaphoreManagerCDT *SemaphoreManagerADT;

// Semaphore manager initialization
void semaphore_manager_init();

// Semaphore operations
int8_t sem_init(sem_t *sem, uint32_t initialValue);
int8_t sem_open(sem_t *sem);
int8_t sem_destroy(sem_t *sem);
int8_t sem_post(sem_t *sem);
int8_t sem_wait(sem_t *sem);

#endif // SEMAPHORE_MANAGER_H

