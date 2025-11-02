#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "test_util.h"

#define SEM_ID 67
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc)
{
  int64_t aux = *p;
  sys_yield(); // This makes the race condition highly probable
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

  // Debug: print process start
  char buf[32];
  uint64_t pid = sys_get_pid();
  puts("Process ");
  puts(itoa((int)pid, buf));
  puts(" starting with inc=");
  puts(itoa((int)inc, buf));
  puts("\n");

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
      sys_sem_wait(SEM_ID);
    slowInc(&global, inc);
    if (use_sem)
      sys_sem_post(SEM_ID);
  }

  if (use_sem)
    sys_sem_close(SEM_ID);

  // Debug: print process end
  puts("Process ");
  puts(itoa((int)pid, buf));
  puts(" finished\n");

  return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[])
{ //{n, use_sem, 0}
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2)
    return -1;

  // added for compatibility with our implementation
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
    pids[i] = sys_create_process(&my_process_inc, argvDec, "my_process_inc", 0, (uint64_t)default_fds);
    pids[i + TOTAL_PAIR_PROCESSES] = sys_create_process(&my_process_inc, argvInc, "my_process_inc", 0, (uint64_t)default_fds);
  }

  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    sys_waitpid(pids[i]);
    sys_waitpid(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  // Parent destroys the semaphore after all children finish
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