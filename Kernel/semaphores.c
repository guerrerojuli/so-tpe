// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <defs.h>
#include <lib.h>
#include <linkedListADT.h>
#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <semaphoreManager.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_SEMAPHORES (1 << 12)

typedef uint16_t sem_t;

typedef struct Semaphore {
	uint32_t value;
	int mutex;					  // 0 libre, 1 ocupado
	LinkedListADT semaphoreQueue; // LinkedListADT de pids
	LinkedListADT mutexQueue;	  // LinkedListADT de pids
} Semaphore;

static Semaphore *createSemaphore(uint32_t initialValue);
static void freeSemaphore(Semaphore *sem);
static void acquireMutex(Semaphore *sem);
static void resumeFirstAvailableProcess(LinkedListADT queue);
static void releaseMutex(Semaphore *sem);
static int up(Semaphore *sem);
static int down(Semaphore *sem);

typedef struct SemaphoreManagerCDT {
	Semaphore *semaphores[MAX_SEMAPHORES];
} SemaphoreManagerCDT;

static SemaphoreManagerCDT semaphore_manager;

void semaphore_manager_init() {
	for (int i = 0; i < MAX_SEMAPHORES; i++)
		semaphore_manager.semaphores[i] = NULL;
}

static SemaphoreManagerADT getSemaphoreManager() {
	return &semaphore_manager;
}

int8_t sem_init(sem_t *sem, uint32_t initialValue) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] != NULL)
		return -1;
	
	semManager->semaphores[id] = createSemaphore(initialValue);
	return 0;
}

int8_t sem_open(sem_t *sem) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES)
		return -1;
	
	return (semManager->semaphores[id] == NULL) ? -1 : 0;
}

int8_t sem_close(sem_t *sem) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;
	
	return 0;
}

int8_t sem_destroy(sem_t *sem) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;

	freeSemaphore(semManager->semaphores[id]);
	semManager->semaphores[id] = NULL;
	return 0;
}

int8_t sem_post(sem_t *sem) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;
	
	return up(semManager->semaphores[id]);
}

int8_t sem_wait(sem_t *sem) {
	if (sem == NULL)
		return -1;
	
	SemaphoreManagerADT semManager = getSemaphoreManager();
	uint16_t id = *sem;
	
	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;
	
	return down(semManager->semaphores[id]);
}

static Semaphore *createSemaphore(uint32_t initialValue) {
	Semaphore *sem = (Semaphore *) mm_alloc(sizeof(Semaphore));
	sem->value = initialValue;
	sem->mutex = 0;
	sem->semaphoreQueue = createLinkedListADT();
	sem->mutexQueue = createLinkedListADT();
	return sem;
}

static void freeSemaphore(Semaphore *sem) {
	freeLinkedListADTDeep(sem->semaphoreQueue);
	freeLinkedListADTDeep(sem->mutexQueue);
	mm_free(sem);
}

static void acquireMutex(Semaphore *sem) {
	while (_xchg(&(sem->mutex), 1)) {
		uint16_t pid = get_pid();
		appendElement(sem->mutexQueue, (void *) ((uint64_t) pid));
		set_status(pid, BLOCKED);
		yield();
	}
}

static int process_is_alive(uint16_t pid) {
	// Simple check - processes are alive if they exist in the scheduler
	// You may need to expand this based on your scheduler implementation
	return pid > 0;  // For now, assume all non-zero PIDs are alive
}

static void resumeFirstAvailableProcess(LinkedListADT queue) {
	Node *current;
	while ((current = getFirst(queue)) != NULL) {
		removeNode(queue, current);
		uint16_t pid = (uint16_t) ((uint64_t) current->data);
		mm_free(current);
		if (process_is_alive(pid)) {
			set_status(pid, READY);
			break;
		}
	}
}

static void releaseMutex(Semaphore *sem) {
	resumeFirstAvailableProcess(sem->mutexQueue);
	sem->mutex = 0;
}

static int up(Semaphore *sem) {
	acquireMutex(sem);
	sem->value++;
	if (sem->value == 0) {
		releaseMutex(sem);
		return -1;
	}
	resumeFirstAvailableProcess(sem->semaphoreQueue);
	releaseMutex(sem);
	// yield();
	return 0;
}

static int down(Semaphore *sem) {
	acquireMutex(sem);
	while (sem->value == 0) {
		uint16_t pid = get_pid();
		appendElement(sem->semaphoreQueue, (void *) ((uint64_t) pid));
		set_status(pid, BLOCKED);
		releaseMutex(sem);
		yield();

		acquireMutex(sem);
	}
	sem->value--;
	releaseMutex(sem);

	return 0;
}