#include <stdint.h>
#include <stddef.h>

#define MCSOS_M0_MAGIC 0x4D435330u  /* "MCS0" */

struct mcsos_smoke_record {
    uint32_t magic;
    uint32_t version;
    size_t size_width;
    uintptr_t pointer_width;
};

__attribute__((used))
const struct mcsos_smoke_record mcsos_smoke_record = {
    .magic = MCSOS_M0_MAGIC,
    .version = 260502u,
    .size_width = sizeof(size_t),
    .pointer_width = sizeof(void*)
};

int mcsos_add(int a, int b) {
    return a + b;
}
