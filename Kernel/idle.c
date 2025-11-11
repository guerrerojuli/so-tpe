

extern void _hlt();

int idle_process(int argc, char **argv)
{
    while (1)
    {
        _hlt();
    }
    return 0;
}
