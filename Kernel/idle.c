// IDLE process - runs when no other process is ready

extern void _hlt();

int idle_process(int argc, char **argv) {
    while (1) {
        _hlt();  // Halt CPU until next interrupt
    }
    return 0;
}
