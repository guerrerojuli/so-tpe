#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#define TEST_PIPE_ID 200
#define READ 0
#define WRITE 1

// Writer process function
static int writer_process(int argc, char **argv) {
    // Open pipe for writing
    if (sys_pipe_open(TEST_PIPE_ID, WRITE) < 0) {
        puts("Writer: Failed to open pipe\n");
        return -1;
    }

    char *message = "Hello from writer process!";
    puts("Writer: Sending message...\n");

    // Write to pipe (using FD)
    sys_write(TEST_PIPE_ID, message, strlen(message) + 1);

    // Close pipe
    sys_pipe_close(TEST_PIPE_ID);
    puts("Writer: Done\n");

    return 0;
}

// Reader process function
static int reader_process(int argc, char **argv) {
    // Open pipe for reading
    if (sys_pipe_open(TEST_PIPE_ID, READ) < 0) {
        puts("Reader: Failed to open pipe\n");
        return -1;
    }

    char buffer[100];
    puts("Reader: Waiting for message...\n");

    // Read from pipe (this will block until data is available)
    int bytes_read = sys_read(TEST_PIPE_ID, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        puts("Reader: Received message: ");
        puts(buffer);
        puts("\n");
    } else {
        puts("Reader: Failed to read from pipe\n");
    }

    // Close pipe
    sys_pipe_close(TEST_PIPE_ID);
    puts("Reader: Done\n");

    return 0;
}

// Main test function
int test_pipes(int argc, char **argv) {
    puts("=== Pipe Test Starting ===\n");
    puts("Creating two processes that will communicate via pipe 200\n\n");

    int16_t default_fds[3] = {0, 1, 2};  // stdin, stdout, stderr

    // Create reader process first
    int16_t reader_pid = sys_create_process(
        (uint64_t)reader_process,
        (uint64_t)0,  // argc
        (uint64_t)NULL,  // argv
        4,  // priority
        (uint64_t)default_fds
    );

    if (reader_pid < 0) {
        puts("Failed to create reader process\n");
        return -1;
    }

    char buf[32];
    puts("Reader process created with PID: ");
    puts(itoa((int)reader_pid, buf));
    puts("\n");

    // Create writer process
    int16_t writer_pid = sys_create_process(
        (uint64_t)writer_process,
        (uint64_t)0,  // argc
        (uint64_t)NULL,  // argv
        4,  // priority
        (uint64_t)default_fds
    );

    if (writer_pid < 0) {
        puts("Failed to create writer process\n");
        return -1;
    }

    puts("Writer process created with PID: ");
    puts(itoa((int)writer_pid, buf));
    puts("\n");

    // Wait for both processes to complete
    puts("\nWaiting for processes to complete...\n");
    sys_waitpid(reader_pid);
    sys_waitpid(writer_pid);

    puts("\n=== Pipe Test Completed Successfully! ===\n");
    return 0;
}
