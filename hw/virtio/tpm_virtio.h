#ifndef QEMU_TPM_VIRTIO_H
#define QEMU_TPM_VIRTIO_H

#include "standard-headers/linux/virtio_tpm.h"
#include "hw/virtio/virtio.h"
#include "hw/tpm/tpm_ppi.h"
#include "hw/pci/pci.h"
#include "sysemu/tpm.h"
#include "sysemu/tpm_backend.h" 
// contains the TPMBackend for TPM full-emulation.

#define TPM_TIS_NUM_LOCALITIES      5     /* per spec */
#define TPM_TIS_LOCALITY_SHIFT      12
#define TPM_TIS_NO_LOCALITY         0xff

#define TPM_TIS_IS_VALID_LOCTY(x)   ((x) < TPM_TIS_NUM_LOCALITIES)

#define TPM_TIS_BUFFER_MAX          4096

typedef enum {
    TPM_TIS_STATE_IDLE = 0,
    TPM_TIS_STATE_READY,
    TPM_TIS_STATE_COMPLETION,
    TPM_TIS_STATE_EXECUTION,
    TPM_TIS_STATE_RECEPTION,
} TPMTISState;

/* locality data  -- all fields are persisted */
typedef struct TPMLocality {
    TPMTISState state;
    uint8_t access;
    uint32_t sts;
    uint32_t iface_id;
    uint32_t inte;
    uint32_t ints;
} TPMLocality;

typedef struct TPMState {
    MemoryRegion mmio;

    unsigned char buffer[TPM_TIS_BUFFER_MAX];
    uint16_t rw_offset;

    uint8_t active_locty;
    uint8_t aborting_locty;
    uint8_t next_locty;

    TPMLocality loc[TPM_TIS_NUM_LOCALITIES];

    qemu_irq irq;
    uint32_t irq_num;

    TPMBackendCmd cmd;

    TPMBackend *be_driver;
    TPMVersion be_tpm_version;

    size_t be_buffer_size;

    bool ppi_enabled;
    TPMPPI ppi;
} TPMState;


OBJECT_DECLARE_SIMPLE_TYPE(VirtIOTPM, TPM_VIRTIO)
#define TPM_VIRTIO(obj) \
        OBJECT_CHECK(VirtIOTPM, (obj), TYPE_TPM_VIRTIO)

// typedef struct VirtIOTPMConfig {
    
//     TPMBackend *tpm;

// } VirtIOTPMConfig;


struct VirtIOTPM {
    VirtIODevice parent_obj;

    /* 2 vqs for request and reponse. */
    VirtQueue *request_vq;

    bool ppi_enabled;

//    VirtIOTPMConfig conf;

    TPMBackend *tpm;

    TPMBackendCmd cmd;
    
    TPMState state;

    // VMChangeStateEntry *vmstate;
};

DECLARE_INSTANCE_CHECKER(VirtIOTPM, VirtIO_TPM,
                         TYPE_TPM_VIRTIO)

#endif