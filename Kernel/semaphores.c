// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <lib.h>
#include <list.h>
#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <semaphores.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Semaphore
{
	uint32_t value;
	int mutex;
	List *semaphoreQueue;
	List *mutexQueue;
} Semaphore;

static Semaphore *create_semaphore(uint32_t initialValue);
static void free_semaphore(Semaphore *sem);
static int process_is_alive(uint16_t pid);
static void acquire_mutex(Semaphore *sem);
static void resume_first_available_process(List *queue);
static void release_mutex(Semaphore *sem);
static int up(Semaphore *sem);
static int down(Semaphore *sem);

typedef struct SemaphoreManagerCDT
{
	Semaphore *semaphores[MAX_SEMAPHORES];
} SemaphoreManagerCDT;

static SemaphoreManagerCDT semaphore_manager;

void semaphore_manager_init()
{
	for (int i = 0; i < MAX_SEMAPHORES; i++)
		semaphore_manager.semaphores[i] = NULL;
}

static SemaphoreManagerADT get_semaphore_manager()
{
	return &semaphore_manager;
}

int8_t sem_init(sem_t *sem, uint32_t initialValue)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] != NULL)
		return -1;

	semManager->semaphores[id] = create_semaphore(initialValue);
	if (semManager->semaphores[id] == NULL)
	{
		return -1;
	}
	return 0;
}

int8_t sem_open(sem_t *sem)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES)
		return -1;

	return (semManager->semaphores[id] == NULL) ? -1 : 0;
}

int8_t sem_close(sem_t *sem)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;

	return 0;
}

int8_t sem_destroy(sem_t *sem)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;

	free_semaphore(semManager->semaphores[id]);
	semManager->semaphores[id] = NULL;
	return 0;
}

int8_t sem_post(sem_t *sem)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;

	return up(semManager->semaphores[id]);
}

int8_t sem_wait(sem_t *sem)
{
	if (sem == NULL)
		return -1;

	SemaphoreManagerADT semManager = get_semaphore_manager();
	uint16_t id = *sem;

	if (id >= MAX_SEMAPHORES || semManager->semaphores[id] == NULL)
		return -1;

	return down(semManager->semaphores[id]);
}

static Semaphore *create_semaphore(uint32_t initialValue)
{
	Semaphore *sem = (Semaphore *)mm_alloc(sizeof(Semaphore));
	if (sem == NULL)
	{
		return NULL;
	}
	sem->value = initialValue;
	sem->mutex = 0;

	sem->semaphoreQueue = (List *)mm_alloc(sizeof(List));
	if (sem->semaphoreQueue == NULL)
	{
		mm_free(sem);
		return NULL;
	}
	list_init(sem->semaphoreQueue);

	sem->mutexQueue = (List *)mm_alloc(sizeof(List));
	if (sem->mutexQueue == NULL)
	{
		mm_free(sem->semaphoreQueue);
		mm_free(sem);
		return NULL;
	}
	list_init(sem->mutexQueue);

	return sem;
}

static void free_semaphore(Semaphore *sem)
{
	if (!sem)
		return;

	if (sem->semaphoreQueue)
	{
		Node *current = sem->semaphoreQueue->head;
		while (current)
		{
			Node *next = current->next;
			mm_free(current);
			current = next;
		}
		mm_free(sem->semaphoreQueue);
	}

	if (sem->mutexQueue)
	{
		Node *current = sem->mutexQueue->head;
		while (current)
		{
			Node *next = current->next;
			mm_free(current);
			current = next;
		}
		mm_free(sem->mutexQueue);
	}

	mm_free(sem);
}

static void acquire_mutex(Semaphore *sem)
{
	while (_xchg(&(sem->mutex), 1))
	{
		uint16_t pid = get_pid();
		Node *node = list_append(sem->mutexQueue, (void *)((uint64_t)pid));
		if (node == NULL)
		{
			continue;
		}
		set_status(pid, BLOCKED);
		yield();
	}
}

static int process_is_alive(uint16_t pid)
{

	return pid > 0;
}

static void resume_first_available_process(List *queue)
{
	Node *current;
	while ((current = list_get_first(queue)) != NULL)
	{

		void *data = list_remove(queue, current);
		uint16_t pid = (uint16_t)((uint64_t)data);
		if (process_is_alive(pid))
		{
			set_status(pid, READY);
			break;
		}
	}
}

static void release_mutex(Semaphore *sem)
{
	resume_first_available_process(sem->mutexQueue);
	sem->mutex = 0;
}

static int up(Semaphore *sem)
{
	acquire_mutex(sem);
	sem->value++;
	if (sem->value == 0)
	{
		release_mutex(sem);
		return -1;
	}
	resume_first_available_process(sem->semaphoreQueue);
	release_mutex(sem);

	return 0;
}

static int down(Semaphore *sem)
{
	acquire_mutex(sem);
	while (sem->value == 0)
	{
		uint16_t pid = get_pid();
		Node *node = list_append(sem->semaphoreQueue, (void *)((uint64_t)pid));
		if (node == NULL)
		{

			release_mutex(sem);
			return -1;
		}
		set_status(pid, BLOCKED);
		release_mutex(sem);
		yield();

		acquire_mutex(sem);
	}
	sem->value--;
	release_mutex(sem);

	return 0;
}