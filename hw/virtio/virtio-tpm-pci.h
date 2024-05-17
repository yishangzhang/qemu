/*
 * Virtio TPM PCI Device
 */

#ifndef HW_VIRTIO_TPM_PCI_H
#define HW_VIRTIO_TPM_PCI_H

#include "hw/virtio/virtio-pci.h"
#include "tpm_virtio.h"
#include "qom/object.h"


/*
 * virtio-tpm-pci-base: This extends VirtioPCIProxy.
 */
#define TYPE_VIRTIO_TPM_PCI_BASE "virtio-tpm-pci-base"
OBJECT_DECLARE_SIMPLE_TYPE(VirtIOTPMPCIBase, VIRTIO_TPM_PCI_BASE)

struct VirtIOTPMPCIBase {
    VirtIOPCIProxy parent_obj;
    VirtIOTPM *vdev;
};

/* to share between PCI and TPM */
#define DEFINE_VIRTIO_TPM_PCI_PROPERTIES(_state)                \
    DEFINE_PROP_BIT("ioeventfd", _state, flags,                 \
                    VIRTIO_PCI_FLAG_USE_IOEVENTFD_BIT, false),  \
        DEFINE_PROP_UINT32("vectors", _state, nvectors, 3)

#endif /* HW_VIRTIO_TPM_PCI_H */
