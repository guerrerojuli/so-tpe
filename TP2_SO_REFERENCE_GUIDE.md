# TP2_SO Reference Guide for Background Process Output

## Quick Reference: Key Files and Line Numbers

This guide provides exact locations in tp2_so codebase for understanding background process output implementation.

---

## File Locations

### 1. Video Driver (Simple, Direct Output)

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/drivers/video.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `printChar(char c)` | 164-213 | Write single character directly to framebuffer |
| `print(const char *s)` | 215-218 | Write string by calling printChar() repeatedly |
| `videoClear()` | 87-92 | Clear screen and reset cursor |
| `printNewline()` | 149-162 | Handle newline and scrolling |

**Key aspects**:
- Global state: `_XPos`, `_YPos` (lines 60)
- Buffer: `_charBuffer[MAX_RESOLUTION]` (line 65)
- Direct framebuffer: `getPtrToPixel(x, y)` (lines 82-85)

---

### 2. Syscalls Implementation

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/syscalls/syscalls.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `syscall_write()` | 77-95 | Write to STDOUT/STDERR/PIPE |
| `syscallDispatcher()` | 35-52 | Route syscall numbers to handlers |

**Critical code** (lines 85-92):
```c
else if (fdValue == STDOUT || fdValue == STDERR) {
    Color prevColor = getFontColor();
    if (fd == STDERR)
        setFontColor(ERROR_COLOR);
    for (uint64_t i = 0; i < len; i++)
        printChar(sourceBuffer[i]);  // DIRECT OUTPUT
    setFontColor(prevColor);
    return len;
}
```

---

### 3. Shell Implementation

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Userland/SampleCodeModule/programs/shell.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `run_shell()` | 94-112 | Main shell loop |
| `createSingleProcess()` | 114-126 | Create single process (fg or bg) |
| `createPipedProcesses()` | 128-151 | Create two processes with pipe |

**Background process code** (lines 118-120):
```c
if (isInBackground(parser)) {
    int16_t fileDescriptors[] = {DEV_NULL, STDOUT, STDERR};
    createProcessWithFds((void *)commands[index].code, program->params, program->name, 4, fileDescriptors);
}
```

**Foreground process code** (lines 122-125):
```c
else {
    int16_t pid = createProcess((void *)commands[index].code, program->params, program->name, 4);
    waitpid(pid);  // Shell waits
}
```

---

### 4. Process Management

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/processes/process.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `initProcess()` | 27-47 | Initialize process structure |
| `assignFileDescriptor()` | 49-53 | Set file descriptor for process |

**Key code** (lines 44-46):
```c
assignFileDescriptor(process, STDIN, fileDescriptors[STDIN], READ);
assignFileDescriptor(process, STDOUT, fileDescriptors[STDOUT], WRITE);
assignFileDescriptor(process, STDERR, fileDescriptors[STDERR], WRITE);
```

---

### 5. Scheduler

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/processes/scheduler.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `schedule()` | 107-142 | Context switch handler |
| `setPriority()` | 54-68 | Change process priority |
| `setStatus()` | 70-97 | Change process state |

**Scheduling loop** (lines 111-113):
```c
scheduler->remainingQuantum--;
if (!scheduler->qtyProcesses || scheduler->remainingQuantum > 0)
    return prevStackPointer;  // No switch needed
```

---

### 6. Interrupts Assembly

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/asm/interrupts.asm`

| Handler | Lines | Purpose |
|---------|-------|---------|
| `_irq00Handler` | 167-181 | Timer interrupt (IRQ 0) |
| `_syscallHandler` | 204-225 | Syscall handler |
| `_irq01Handler` | 184-185 | Keyboard interrupt |

**Timer interrupt code** (lines 167-181):
```asm
_irq00Handler:
    pushState          ; Save all registers
    mov rdi, 0
    call irqDispatcher ; Timer tick
    mov rdi, rsp
    call schedule      ; Scheduler decides next process
    mov rsp, rax       ; Set new process's stack
    mov al, 20h
    out 20h, al        ; EOI
    popState
    iretq
```

---

### 7. Input Parser

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Userland/SampleCodeModule/programs/inputParserADT.c`

| Function | Lines | Purpose |
|----------|-------|---------|
| `parseInput()` | 23-59 | Parse command line |
| `isInBackground()` | 130-132 | Check if `&` in command |

**Background detection** (lines 53-54):
```c
if (*input == AMPERSAND)
    inputParserADT->background = 1;
```

---

### 8. Process Structure

**File**: `/Users/agu.egea/Desktop/Q2_2025/sistemas_operativos/tp2_so/Kernel/include/process.h` (likely)

**Key fields** (from process.c):
- `pid`: Process ID
- `fileDescriptors[]`: Per-process file descriptors
- `status`: READY, RUNNING, BLOCKED, ZOMBIE
- `priority`: Priority level (0-4)
- `stackPos`: Current stack pointer
- `parentPid`: Parent process ID
- `zombieChildren`: List of child zombies

---

## Key Constants and Values

### File Descriptor Numbers

From `syscalls.c` and process code:
- `STDIN = 0` - Standard input
- `STDOUT = 1` - Standard output
- `STDERR = 2` - Standard error
- `DEV_NULL = -1` - Null device (discards writes)
- `BUILT_IN_DESCRIPTORS = 3` - Pipe FDs start at 3+

### Priority Levels

From `scheduler.c`:
- `MAX_PRIORITY = 4` - Highest priority
- `MIN_PRIORITY = 0` - Lowest priority
- `QTY_READY_LEVELS = 5` - Priority levels (0-4)
- `BLOCKED_INDEX = QTY_READY_LEVELS` - Index for blocked processes

### Screen/Video Constants

From `video.c`:
- `MAX_RESOLUTION = 64 * 128` - Character buffer size
- `FIRST_CHAR` / `LAST_CHAR` - Printable character range
- `_screenData->framebuffer` - VBE framebuffer address
- `_XPos, _YPos` - Global cursor position

---

## Execution Flow Diagrams

### Background Process Creation Flow

```
User types: "loop 5 &"
    ↓
Shell parser sets background=1
    ↓
createSingleProcess() called
    ↓
fileDescriptors[] = {DEV_NULL, STDOUT, STDERR}
    ↓
createProcessWithFds() called
    ↓
Process created with STDOUT = STDOUT (not redirected!)
    ↓
Shell prompt returns immediately (doesn't wait)
```

### Background Process Output Flow

```
Background process calls write(1, buffer, len)
    ↓
sys_write(STDOUT, buffer, len) syscall
    ↓
fdValue = STDOUT
    ↓
for (i=0; i<len; i++) printChar(buffer[i])
    ↓
printChar() writes directly to framebuffer
    ↓
Character visible on screen immediately
    ↓
Timer interrupt switches context
    ↓
Another process runs (foreground or other background)
    ↓
Both outputs interleaved on screen
```

### Scheduling Flow

```
Timer interrupt (IRQ 0) every ~10ms
    ↓
_irq00Handler in interrupts.asm
    ↓
schedule() called in scheduler.c
    ↓
Current process's quantum decremented
    ↓
If quantum expired:
    Remove from ready queue
    Add to back of queue at same priority
    ↓
Select highest priority non-empty queue
    ↓
Get first process from queue
    ↓
Return its stack pointer
    ↓
Assembly switches to new process's stack
    ↓
New process resumes execution
```

---

## Testing Background Processes

### From tp2_so Shell

```bash
# Start background process that outputs
loop 2 &

# Start foreground process
echo "Hello"

# Expected: loop output appears immediately in background
# Then echo output on top
```

### Expected Behavior

1. `loop 2 &` - Process created, shell prompt returns immediately
2. `echo "Hello"` - Shell creates foreground process
3. `Hello` appears on screen
4. Loop process continues running, its output visible between/after commands

---

## Critical Implementation Details

### Why It Works in TP2_SO

1. **Direct Output**: `printChar()` → framebuffer (immediate)
2. **No Buffering**: Each character rendered as syscall executes
3. **No Locking**: Multiple processes can write (races OK)
4. **Shared Console**: Background stdout = STDOUT = foreground
5. **Preemption**: Timer interrupt allows background to run
6. **No Redirection**: Background processes not redirected to pipes

### Contrast with Typical Issues

**If buffering**: Output stuck in buffer until flush
**If locking**: Background blocked waiting for foreground's lock
**If redirected**: Background output goes to pipe, never read
**If no preemption**: Background never gets CPU time
**If small quantum**: Background loses CPU before finishing output

---

## Debugging Checklist

When tracing background output issues:

- [ ] Check `createSingleProcess()` - is background stdout redirected?
- [ ] Check `syscall_write()` - does it buffer or output directly?
- [ ] Check `printChar()` - does it write to framebuffer immediately?
- [ ] Check scheduler - does background get CPU time?
- [ ] Check video driver - any locks/mutexes?
- [ ] Check timer interrupt - is preemption enabled?
- [ ] Check console driver - is output buffered?
- [ ] Check process FDs - are they set correctly?

---

## Commands to Extract from TP2_SO

If implementing background support:

1. Copy `shell.c` background detection logic (lines 118-125)
2. Copy `video.c` printChar() implementation (lines 164-213)
3. Copy `syscalls.c` syscall_write() (lines 77-95)
4. Copy scheduler's preemption logic (scheduler.c, interrupts.asm)
5. Ensure background processes don't redirect STDOUT

---

## Summary of Key Differences

```
TP2_SO Implementation            |  Typical Issue in Other OS
---------------------------------|-----------------------------
No buffering in sys_write()      |  Buffering delays output
Direct to framebuffer            |  Via console driver
Global video state               |  Per-process video state
No locks on output               |  Mutex causes deadlock
Background stdout = STDOUT       |  Redirected to pipe/buffer
Immediate character visibility   |  Delayed rendering
Preemptive scheduler             |  Cooperative scheduler
Timer interrupt every 10ms       |  Infrequent context switch
```

---

## Files to Copy/Reference

Essential files to study from tp2_so:

1. `Kernel/drivers/video.c` - Model your video driver after this
2. `Kernel/syscalls/syscalls.c` - Copy syscall_write() logic
3. `Userland/SampleCodeModule/programs/shell.c` - Copy background handling
4. `Kernel/processes/scheduler.c` - Reference scheduler implementation
5. `Kernel/asm/interrupts.asm` - Reference timer interrupt handling

