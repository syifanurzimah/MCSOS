#include "mcs_sync.h"
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

static mcs_spinlock_t test_spin;
static mcs_mutex_t test_mutex;
static mcs_lockdep_state_t test_lockdep;

void m12_sync_selftest(void)
{
    log_writeln("[M12] sync selftest start");

    mcs_lockdep_init(&test_lockdep);

    mcs_spin_init(
        &test_spin,
        1,
        "m12_test_spin"
    );

    mcs_spin_lock(&test_spin);

    if (!mcs_spin_is_locked(&test_spin)) {
        KERNEL_PANIC("M12 spinlock failed", 0x1201);
    }

    mcs_spin_unlock(&test_spin);

    if (mcs_spin_is_locked(&test_spin)) {
        KERNEL_PANIC("M12 spinlock unlock failed", 0x1202);
    }


    mcs_mutex_init(
        &test_mutex,
        2,
        "m12_test_mutex"
    );

    if (mcs_mutex_try_lock(&test_mutex, 1) != MCS_SYNC_OK) {
        KERNEL_PANIC("M12 mutex lock failed", 0x1203);
    }

    if (!mcs_mutex_is_locked(&test_mutex)) {
        KERNEL_PANIC("M12 mutex state failed", 0x1204);
    }

    if (mcs_mutex_unlock(&test_mutex, 1) != MCS_SYNC_OK) {
        KERNEL_PANIC("M12 mutex unlock failed", 0x1205);
    }

    log_writeln("[M12] sync selftest passed");
}
