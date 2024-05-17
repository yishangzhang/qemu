#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "tpm_virtio.h"
#include "hw/qdev-properties.h"
#include "hw/virtio/virtio.h"
#include "hw/virtio/virtio-bus.h"
#include "hw/virtio/virtio-pci.h"
#include "hw/virtio/virtio-tpm-pci.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qom/object.h"

typedef struct VirtIOTPMPCI VirtIOTPMPCI;

/*
 * virtio-tpm-pci: This extends VirtioPCIProxy.
 */
#define TYPE_VIRTIO_TPM_PCI "tpm-virtio-pci-base"
DECLARE_INSTANCE_CHECKER(VirtIOTPMPCI, VIRTIO_TPM_PCI,
                         TYPE_VIRTIO_TPM_PCI)

struct VirtIOTPMPCI {
    VirtIOPCIProxy parent_obj;
    VirtIOTPM vdev;
};


static void virtio_tpm_pci_realize(VirtIOPCIProxy *vpci_dev, Error **errp)
{
    VirtIOTPMPCI *dev = VIRTIO_TPM_PCI(vpci_dev);
    DeviceState *vdev = DEVICE(&dev->vdev);

    if (!qdev_realize(vdev, BUS(&vpci_dev->bus), errp)) {
        return;
    }
}

static void virtio_tpm_pci_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioPCIClass *k = VIRTIO_PCI_CLASS(klass);
    PCIDeviceClass *pcidev_k = PCI_DEVICE_CLASS(klass);

    k->realize = virtio_tpm_pci_realize;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    
    pcidev_k->vendor_id = PCI_VENDOR_ID_REDHAT_QUMRANET;
    pcidev_k->device_id = PCI_DEVICE_ID_VIRTIO_TPM;
    pcidev_k->revision = VIRTIO_PCI_ABI_VERSION;
    pcidev_k->class_id = PCI_CLASS_OTHERS;
}


static void virtio_tpm_pci_instance_init(Object *obj)
{
    VirtIOTPMPCI *dev = VIRTIO_TPM_PCI(obj);

    virtio_instance_init_common(obj, &dev->vdev, sizeof(dev->vdev),
                                TYPE_TPM_VIRTIO);
}

static const VirtioPCIDeviceTypeInfo virtio_tpm_pci_info = {
    .base_name             = TYPE_VIRTIO_TPM_PCI,
    .generic_name          = "virtio-tpm-pci",
    .transitional_name     = "virtio-tpm-pci-transitional",
    .non_transitional_name = "virtio-tpm-pci-non-transitional",
    .instance_size = sizeof(VirtIOTPMPCI),
    .instance_init = virtio_tpm_pci_instance_init,
    .class_init    = virtio_tpm_pci_class_init,
};

static void virtio_tpm_pci_register(void)
{
    virtio_pci_types_register(&virtio_tpm_pci_info);
}

type_init(virtio_tpm_pci_register)