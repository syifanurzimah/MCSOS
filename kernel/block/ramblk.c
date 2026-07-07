#include "mcsos/block.h"

static void mcsos_memcpy_u8_ram(void *dst, const void *src, uint64_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    for (uint64_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

static mcsos_blk_status_t mcsos_ramblk_read(
    mcsos_blk_device_t *dev,
    uint64_t lba,
    uint32_t count,
    void *buffer
) {
    if (dev == 0 || buffer == 0 || count == 0u) {
        return MCSOS_BLK_EINVAL;
    }

    if (lba + count > dev->block_count) {
        return MCSOS_BLK_ERANGE;
    }

    mcsos_ramblk_t *ram = (mcsos_ramblk_t *)dev->driver_data;

    mcsos_memcpy_u8_ram(
        buffer,
        ram->storage + (lba * dev->block_size),
        (uint64_t)count * dev->block_size
    );

    return MCSOS_BLK_OK;
}

static mcsos_blk_status_t mcsos_ramblk_write(
    mcsos_blk_device_t *dev,
    uint64_t lba,
    uint32_t count,
    void *buffer
) {
    if (dev == 0 || buffer == 0 || count == 0u) {
        return MCSOS_BLK_EINVAL;
    }

    if (lba + count > dev->block_count) {
        return MCSOS_BLK_ERANGE;
    }

    mcsos_ramblk_t *ram = (mcsos_ramblk_t *)dev->driver_data;

    mcsos_memcpy_u8_ram(
        ram->storage + (lba * dev->block_size),
        buffer,
        (uint64_t)count * dev->block_size
    );

    return MCSOS_BLK_OK;
}

static mcsos_blk_status_t mcsos_ramblk_flush(
    mcsos_blk_device_t *dev,
    uint64_t lba,
    uint32_t count,
    void *buffer
) {
    (void)dev;
    (void)lba;
    (void)count;
    (void)buffer;
    return MCSOS_BLK_OK;
}
static const mcsos_blk_ops_t mcsos_ramblk_ops = {
    .read = mcsos_ramblk_read,
    .write = mcsos_ramblk_write,
    .flush = mcsos_ramblk_flush
};

mcsos_blk_status_t mcsos_ramblk_init(
    mcsos_blk_device_t *dev,
    mcsos_ramblk_t *ram,
    const char *name,
    uint8_t *storage,
    uint64_t storage_size,
    uint32_t block_size
) {
    if (dev == 0 || ram == 0 || storage == 0 || name == 0 || block_size == 0u) {
        return MCSOS_BLK_EINVAL;
    }

    if (storage_size % block_size != 0u) {
        return MCSOS_BLK_EINVAL;
    }

    ram->storage = storage;
    ram->storage_size = storage_size;

    uint32_t i = 0;
    while (name[i] != '\0' && i < (MCSOS_BLK_NAME_MAX - 1u)) {
        dev->name[i] = name[i];
        i++;
    }
    dev->name[i] = '\0';

    dev->block_size = block_size;
    dev->block_count = storage_size / block_size;
    dev->flags = 0u;
    dev->ops = &mcsos_ramblk_ops;
    dev->driver_data = ram;

    return MCSOS_BLK_OK;
}
