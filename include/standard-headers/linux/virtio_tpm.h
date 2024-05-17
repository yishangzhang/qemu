#ifndef _LINUX_VIRTIO_TPM_H
#define _LINUX_VIRTIO_TPM_H
#include "standard-headers/linux/types.h"
#include "standard-headers/linux/virtio_types.h"
#include "standard-headers/linux/virtio_ids.h"
#include "standard-headers/linux/virtio_config.h"

/* The feature bitmap for virtio tpm */
#define VIRTIO_TPM_F_CAN_PRINT 0

// TODO: Useless
struct virtio_tpm_config {
    /* Number of pages host wants Guest to give up. */
    __u32 num_pages;
    /* Number of pages we've actually got in balloon. */
    __u32 actual;
};

struct virtio_tpm_stat {
    __virtio16 tag;
    __virtio64 val;
} __attribute__((packed));

#endif /* _LINUX_VIRTIO_TPM_H */