// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 
 
 
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "commands.h"

#define MAX_WRITERS 26  

#define MVAR_MUTEX 100
#define MVAR_READ_SEM 101
#define MVAR_WRITE_SEM 102

#define STDIN 0
#define STDOUT 1
#define STDERR 2

 
static char shared_mvar = 0;

 
static void build_name(char *dest, const char *prefix, int num)
{
    char num_str[12];  

     
    strcpy(dest, prefix);

     
    itoa(num, num_str);

     
    strcat(dest, num_str);
}

 
static void busy_wait(uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
        ;
}

 
static uint64_t writer_process(uint64_t argc, char *argv[])
{
    if (argc != 1)
        return -1;

    int writer_id = atoi(argv[0]) % MAX_WRITERS;
    char my_letter = 'A' + writer_id;

     
    if (sys_sem_open(MVAR_MUTEX) < 0 ||
        sys_sem_open(MVAR_READ_SEM) < 0 ||
        sys_sem_open(MVAR_WRITE_SEM) < 0)
    {
        puts("Writer: ERROR opening semaphores\n");
        return -1;
    }

     
    while (1)  
    {
         
        uint32_t wait_time = rand() % 100000;
        busy_wait(wait_time);

         
        if (sys_sem_wait(MVAR_WRITE_SEM) < 0)
        {
            puts("Writer: ERROR in sem_wait(MVAR_WRITE_SEM)\n");
            return -1;
        }

         
        if (sys_sem_wait(MVAR_MUTEX) < 0)
        {
            puts("Writer: ERROR in sem_wait(MVAR_MUTEX)\n");
            return -1;
        }

         
        shared_mvar = my_letter;

         
        if (sys_sem_post(MVAR_MUTEX) < 0)
        {
            puts("Writer: ERROR in sem_post(MVAR_MUTEX)\n");
            return -1;
        }

         
        if (sys_sem_post(MVAR_READ_SEM) < 0)
        {
            puts("Writer: ERROR in sem_post(MVAR_READ_SEM)\n");
            return -1;
        }
    }

    return 0;
}

 
static uint64_t reader_process(uint64_t argc, char *argv[])
{
    if (argc != 1)
        return -1;

    int reader_id = atoi(argv[0]);

     
    if (sys_sem_open(MVAR_MUTEX) < 0 ||
        sys_sem_open(MVAR_READ_SEM) < 0 ||
        sys_sem_open(MVAR_WRITE_SEM) < 0)
    {
        puts("Reader: ERROR opening semaphores\n");
        return -1;
    }

     
    while (1)  
    {
         
        uint32_t wait_time = rand() % 100000;
        busy_wait(wait_time);

         
        if (sys_sem_wait(MVAR_READ_SEM) < 0)
        {
            puts("Reader: ERROR in sem_wait(MVAR_READ_SEM)\n");
            return -1;
        }

         
        if (sys_sem_wait(MVAR_MUTEX) < 0)
        {
            puts("Reader: ERROR in sem_wait(MVAR_MUTEX)\n");
            return -1;
        }

         
        char value = shared_mvar;

         
        if (sys_sem_post(MVAR_MUTEX) < 0)
        {
            puts("Reader: ERROR in sem_post(MVAR_MUTEX)\n");
            return -1;
        }

         
        if (sys_sem_post(MVAR_WRITE_SEM) < 0)
        {
            puts("Reader: ERROR in sem_post(MVAR_WRITE_SEM)\n");
            return -1;
        }

         
         
        void *args[] = {(void *)&reader_id, (void *)&value};
        printf(" [%d]%c ", args);
    }

    return 0;
}

static int mvar_func(int argc, char **argv)
{
     
    if (argc != 3)
    {
        puts("Usage: mvar <num_writers> <num_readers>\n");
        return -1;
    }

    int num_writers = atoi(argv[1]);
    int num_readers = atoi(argv[2]);

    if (num_writers <= 0 || num_readers <= 0)
    {
        puts("Error: num_writers and num_readers must be positive\n");
        return -1;
    }

    if (num_writers > MAX_WRITERS)
    {
        puts("Error: max 26 writers (A-Z)\n");
        return -1;
    }

     
    if (sys_sem_init(MVAR_MUTEX, 1) < 0)
    {
        puts("mvar: ERROR creating MVAR_MUTEX\n");
        return -1;
    }

    if (sys_sem_init(MVAR_READ_SEM, 0) < 0)
    {
        puts("mvar: ERROR creating MVAR_READ_SEM\n");
        sys_sem_destroy(MVAR_MUTEX);
        return -1;
    }

    if (sys_sem_init(MVAR_WRITE_SEM, 1) < 0)
    {
        puts("mvar: ERROR creating MVAR_WRITE_SEM\n");
        sys_sem_destroy(MVAR_MUTEX);
        sys_sem_destroy(MVAR_READ_SEM);
        return -1;
    }

     
    int16_t default_fds[3] = {STDIN, STDOUT, STDERR};

     
    for (int i = 0; i < num_writers; i++)
    {
        char id_str[12];
        itoa(i, id_str);
        char *writer_args[] = {id_str, NULL};

        char writer_name[32];
        build_name(writer_name, "writer_", i);

        int64_t pid = sys_create_process((uint64_t)&writer_process, (uint64_t)writer_args, (uint64_t)writer_name, 2, (uint64_t)default_fds);
        if (pid < 0)
        {
            puts("mvar: ERROR creating writer process\n");
            sys_sem_destroy(MVAR_MUTEX);
            sys_sem_destroy(MVAR_READ_SEM);
            sys_sem_destroy(MVAR_WRITE_SEM);
            return -1;
        }
    }

     
    for (int i = 0; i < num_readers; i++)
    {
        char id_str[12];
        itoa(i, id_str);
        char *reader_args[] = {id_str, NULL};

        char reader_name[32];
        build_name(reader_name, "reader_", i);

        int64_t pid = sys_create_process((uint64_t)&reader_process, (uint64_t)reader_args, (uint64_t)reader_name, 2, (uint64_t)default_fds);
        if (pid < 0)
        {
            puts("mvar: ERROR creating reader process\n");
            sys_sem_destroy(MVAR_MUTEX);
            sys_sem_destroy(MVAR_READ_SEM);
            sys_sem_destroy(MVAR_WRITE_SEM);
            return -1;
        }
    }

    return 0;
}

command mvar_cmd = {
    "mvar",
    mvar_func,
    "Multiple readers/writers with MVar pattern"};
