#ifndef QEMU_TPM_VIRTIO_H
#define QEMU_TPM_VIRTIO_H

#include "standard-headers/linux/virtio_tpm.h"
#include "hw/virtio/virtio.h"
#include "hw/pci/pci.h"
#include "sysemu/tpm.h"
#include "sysemu/tpm_backend.h" 
#include "hw/virtio/virtio-tpm-pci.h"
// contains the TPMBackend for TPM full-emulation.


OBJECT_DECLARE_SIMPLE_TYPE(VirtIOTPM, TPM_VIRTIO)
#define TPM_VIRTIO(obj) \
        OBJECT_CHECK(VirtIOTPM, (obj), TYPE_TPM_VIRTIO)

typedef struct VirtIOTPMConfig {
    
    TPMBackend *tpm;

} VirtIOTPMConfig;


struct VirtIOTPM {
    VirtIODevice parent_obj;

    /* 2 vqs for request and reponse. */
    VirtQueue *request_vq, *response_vq;

    bool ppi_enabled;

    VirtIOTPMConfig conf;

    TPMBackend *tpm;

    TPMBackendCmd cmd;
    
    // TPMState *state;

    // VMChangeStateEntry *vmstate;
};

DECLARE_INSTANCE_CHECKER(VirtIOTPM, VirtIO_TPM,
                         TYPE_TPM_VIRTIO)

#endif