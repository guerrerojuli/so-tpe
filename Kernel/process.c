#include <stdint.h>
#include <scheduler.h>
#include <task.h>
#include <videoDriver.h>
#include <time.h>

static uint8_t procA_stack[4096] __attribute__((aligned(16)));
static uint8_t procB_stack[4096] __attribute__((aligned(16)));

static void sleep_1s(void)
{
    int start = seconds_elapsed();
    while (seconds_elapsed() - start < 1)
        ;
}

static void procA(void)
{
    const char A = 'A';
    while (1)
    {
        console_write(&A, 1, 0xFFFFFF);
        sleep_1s();
    }
}

static void procB(void)
{
    const char B = 'B';
    while (1)
    {
        console_write(&B, 1, 0xFFFFFF);
        sleep_1s();
    }
}

static uint64_t build_initial_rsp(void (*entry)(void), void *stack_top)
{
    // Matches scheduler/ASM return path: [ ... 15 regs ... | RIP ]
    uint64_t *sp = (uint64_t *)stack_top;
    *(--sp) = (uint64_t)entry; // return RIP for 'ret'
    for (int i = 0; i < 15; i++)
        *(--sp) = 0;
    return (uint64_t)sp;
}

void process_create_demo_tasks(void)
{
    uint64_t rspA = build_initial_rsp(procA, procA_stack + sizeof(procA_stack));
    uint64_t rspB = build_initial_rsp(procB, procB_stack + sizeof(procB_stack));

    scheduler_add_task(rspA, procA);
    scheduler_add_task(rspB, procB);
}