#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "commands.h"

// Forward declaration from test_pipes.c
extern int test_pipes(int argc, char **argv);

static int test_pipes_func(int argc, char **argv) {
    // Call test_pipes with argv
    return test_pipes(argc - 1, &argv[1]);
}

command test_pipes_cmd = {
    "test-pipes",
    test_pipes_func,
    "Run pipe communication test",
    "TEST-PIPES(1)               User Commands               TEST-PIPES(1)\n\n"
    "NAME\n"
    "       test-pipes - test inter-process communication using pipes\n\n"
    "SYNOPSIS\n"
    "       test-pipes\n\n"
    "DESCRIPTION\n"
    "       Tests IPC (Inter-Process Communication) using unidirectional pipes.\n"
    "       Creates two processes: a writer and a reader that communicate via\n"
    "       pipe 200. The writer sends a message and the reader receives it.\n\n"
    "       This test demonstrates:\n"
    "       - Pipe creation and opening for read/write\n"
    "       - Blocking read/write operations\n"
    "       - Transparent I/O (same read/write for terminal and pipes)\n"
    "       - Process synchronization via pipes\n\n"
    "EXAMPLES\n"
    "       test-pipes    # Run the pipe communication test\n\n"
    "EXPECTED OUTPUT\n"
    "       === Pipe Test Starting ===\n"
    "       Creating two processes that will communicate via pipe 200\n"
    "       Reader process created with PID: <pid>\n"
    "       Writer process created with PID: <pid>\n"
    "       Waiting for processes to complete...\n"
    "       Reader: Waiting for message...\n"
    "       Writer: Sending message...\n"
    "       Writer: Done\n"
    "       Reader: Received message: Hello from writer process!\n"
    "       Reader: Done\n"
    "       === Pipe Test Completed Successfully! ===\n\n"
};
