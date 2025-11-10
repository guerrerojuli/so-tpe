// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// IDLE process - runs when no other process is ready

extern void _hlt();

int idle_process(int argc, char **argv) {
    while (1) {
        _hlt();  // Halt CPU until next interrupt
    }
    return 0;
}
