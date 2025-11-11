// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//-V:printf:111,576,618,719,303
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "commands.h"

#define MAX_WRITERS 26 // A-Z

#define MVAR_MUTEX 100
#define MVAR_READ_SEM 101
#define MVAR_WRITE_SEM 102

#define STDIN 0
#define STDOUT 1
#define STDERR 2

// Shared MVar variable
static char shared_mvar = 0;

// Build a string with prefix and numeric suffix (e.g., "writer_3")
static void build_name(char *dest, const char *prefix, int num)
{
    char num_str[12]; // Enough for 32-bit int

    // Copy prefix
    strcpy(dest, prefix);

    // Convert number to string
    itoa(num, num_str);

    // Concatenate number
    strcat(dest, num_str);
}

// Simple busy wait function
static void busy_wait(uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
        ;
}

// Writer process - writes unique letters to the MVar
static uint64_t writer_process(uint64_t argc, char *argv[])
{
    if (argc != 1)
        return -1;

    int writer_id = atoi(argv[0]) % MAX_WRITERS;
    char my_letter = 'A' + writer_id;

    // Open semaphores
    if (sys_sem_open(MVAR_MUTEX) < 0 ||
        sys_sem_open(MVAR_READ_SEM) < 0 ||
        sys_sem_open(MVAR_WRITE_SEM) < 0)
    {
        puts("Writer: ERROR opening semaphores\n");
        return -1;
    }

    // Infinite loop: write to MVar
    while (1) //-V776
    {
        // Random active wait
        uint32_t wait_time = rand() % 100000;
        busy_wait(wait_time);

        // Wait until can write (MVar is empty)
        if (sys_sem_wait(MVAR_WRITE_SEM) < 0)
        {
            puts("Writer: ERROR in sem_wait(MVAR_WRITE_SEM)\n");
            return -1;
        }

        // Lock mutex
        if (sys_sem_wait(MVAR_MUTEX) < 0)
        {
            puts("Writer: ERROR in sem_wait(MVAR_MUTEX)\n");
            return -1;
        }

        // Write to shared variable
        shared_mvar = my_letter;

        // Unlock mutex
        if (sys_sem_post(MVAR_MUTEX) < 0)
        {
            puts("Writer: ERROR in sem_post(MVAR_MUTEX)\n");
            return -1;
        }

        // Signal readers (MVar is full)
        if (sys_sem_post(MVAR_READ_SEM) < 0)
        {
            puts("Writer: ERROR in sem_post(MVAR_READ_SEM)\n");
            return -1;
        }
    }

    return 0;
}

// Reader process - reads values from the MVar
static uint64_t reader_process(uint64_t argc, char *argv[])
{
    if (argc != 1)
        return -1;

    int reader_id = atoi(argv[0]);

    // Open semaphores
    if (sys_sem_open(MVAR_MUTEX) < 0 ||
        sys_sem_open(MVAR_READ_SEM) < 0 ||
        sys_sem_open(MVAR_WRITE_SEM) < 0)
    {
        puts("Reader: ERROR opening semaphores\n");
        return -1;
    }

    // Infinite loop: read from MVar
    while (1) //-V776
    {
        // Random active wait
        uint32_t wait_time = rand() % 100000;
        busy_wait(wait_time);

        // Wait until data is available (MVar is full)
        if (sys_sem_wait(MVAR_READ_SEM) < 0)
        {
            puts("Reader: ERROR in sem_wait(MVAR_READ_SEM)\n");
            return -1;
        }

        // Lock mutex
        if (sys_sem_wait(MVAR_MUTEX) < 0)
        {
            puts("Reader: ERROR in sem_wait(MVAR_MUTEX)\n");
            return -1;
        }

        // Read from shared variable
        char value = shared_mvar;

        // Unlock mutex
        if (sys_sem_post(MVAR_MUTEX) < 0)
        {
            puts("Reader: ERROR in sem_post(MVAR_MUTEX)\n");
            return -1;
        }

        // Signal writers (MVar is empty)
        if (sys_sem_post(MVAR_WRITE_SEM) < 0)
        {
            puts("Reader: ERROR in sem_post(MVAR_WRITE_SEM)\n");
            return -1;
        }

        // Print value with identifier
        // Using different output formats for different readers
        void *args[] = {(void *)&reader_id, (void *)&value};
        printf(" [%d]%c ", args);
    }

    return 0;
}

static int mvar_func(int argc, char **argv)
{
    // argv[0] = "mvar", argv[1] = num_writers, argv[2] = num_readers
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

    // Initialize semaphores
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

    // Default file descriptors
    int16_t default_fds[3] = {STDIN, STDOUT, STDERR};

    // Create writer processes
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

    // Create reader processes
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
