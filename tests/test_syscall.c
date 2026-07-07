#include <assert.h>
#include <stdint.h>

#include "mcsos/syscall.h"

static uint64_t fake_ticks(void) {
    return 12345u;
}

static int yield_called = 0;
static void fake_yield(void) {
    yield_called++;
}

static int exit_called = 0;
static int exit_code = -1;
static void fake_exit(int code) {
    exit_called++;
    exit_code = code;
}

static int64_t fake_serial(const char *buf, size_t len) {
    (void)buf;
    return (int64_t)len;
}

int main(void) {
    mcsos_syscall_ops_t ops = {
        .get_ticks = fake_ticks,
        .yield_current = fake_yield,
        .exit_current = fake_exit,
        .write_serial = fake_serial
    };

    mcsos_syscall_init(&ops);

    char buffer[32] = "hello";
    mcsos_user_region_t region = {
        .base = (uintptr_t)buffer,
        .limit = (uintptr_t)(buffer + sizeof(buffer))
    };
    mcsos_syscall_set_user_region(region);

    assert(mcsos_syscall_dispatch(
        MCSOS_SYS_PING,
        0,0,0,0,0,0) == 0x2605020AL);

    assert(mcsos_syscall_dispatch(
        MCSOS_SYS_GET_TICKS,
        0,0,0,0,0,0) == 12345);

    assert(mcsos_syscall_dispatch(
        MCSOS_SYS_WRITE_SERIAL,
        (uint64_t)(uintptr_t)buffer,
        5,0,0,0,0) == 5);

    assert(mcsos_syscall_dispatch(
        MCSOS_SYS_YIELD,
        0,0,0,0,0,0) == MCSOS_OK);

    assert(yield_called == 1);

    assert(mcsos_syscall_dispatch(
        MCSOS_SYS_EXIT_THREAD,
        99,0,0,0,0,0) == MCSOS_OK);

    assert(exit_called == 1);
    assert(exit_code == 99);

    assert(mcsos_syscall_dispatch(
        999,0,0,0,0,0,0) == MCSOS_ENOSYS);

    return 0;
}
