#define BUDDY
#include <stdint.h>
#include "../Kernel/include/memoryManager.h"

// Globals for BUDDY
zone_t buddy_zone;
static page_t buddy_pages[2048];

// Include the implementation
#include "../Kernel/memory/buddyMemory.c"
