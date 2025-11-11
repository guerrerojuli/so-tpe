 
 
#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "test_util.h"

#define SEM_ID 67
#define TOTAL_PAIR_PROCESSES 2

int64_t global;  

void slowInc(int64_t *p, int64_t inc)
{
  int64_t aux = *p;
  sys_yield();  
  aux += inc;
  *p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[])
{
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 3)
    return -1;

  if ((n = satoi(argv[0])) <= 0)
    return -1;
  if ((inc = satoi(argv[1])) == 0)
    return -1;
  if ((use_sem = satoi(argv[2])) < 0)
    return -1;

  if (use_sem)
    if (sys_sem_open(SEM_ID) < 0)
    {
      puts("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++)
  {
    if (use_sem)
    {
      if (sys_sem_wait(SEM_ID) < 0)
      {
        puts("test_sync: ERROR in sem_wait\n");
        return -1;
      }
    }
    slowInc(&global, inc);
    if (use_sem)
    {
      if (sys_sem_post(SEM_ID) < 0)
      {
        puts("test_sync: ERROR in sem_post\n");
        return -1;
      }
    }
  }

  if (use_sem)
    sys_sem_close(SEM_ID);

  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[])
{  
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2)
    return -1;

   
  int8_t useSem = satoi(argv[1]);
  if (useSem)
  {
    if (sys_sem_init(SEM_ID, 1) < 0)
    {
      puts("test_sync: ERROR creating semaphore\n");
      return -1;
    }
  }

  char *argvDec[] = {argv[0], "-1", argv[1], NULL};
  char *argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  int16_t default_fds[3] = {STDIN, STDOUT, STDERR};

  uint64_t i;
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    pids[i] = sys_create_process((uint64_t)&my_process_inc, (uint64_t)argvDec, (uint64_t)"my_process_inc", 0, (uint64_t)default_fds);
    if (pids[i] < 0)
    {
      puts("test_sync: ERROR creating decrement process\n");
      if (useSem)
        sys_sem_destroy(SEM_ID);
      return -1;
    }
    pids[i + TOTAL_PAIR_PROCESSES] = sys_create_process((uint64_t)&my_process_inc, (uint64_t)argvInc, (uint64_t)"my_process_inc", 0, (uint64_t)default_fds);
    if (pids[i + TOTAL_PAIR_PROCESSES] < 0)
    {
      puts("test_sync: ERROR creating increment process\n");
      if (useSem)
        sys_sem_destroy(SEM_ID);
      return -1;
    }
  }

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    if (sys_waitpid(pids[i]) < 0)
    {
      puts("test_sync: ERROR waiting for decrement process\n");
    }
    if (sys_waitpid(pids[i + TOTAL_PAIR_PROCESSES]) < 0)
    {
      puts("test_sync: ERROR waiting for increment process\n");
    }
  }

   
  if (useSem)
  {
    sys_sem_destroy(SEM_ID);
  }

  char buf[32];
  puts("Final value: \n");
  puts(itoa((int)global, buf));
  puts("\n");

  return 0;
}