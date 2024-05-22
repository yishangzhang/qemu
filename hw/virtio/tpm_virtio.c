/*
 * A virtio device implementing a hardware random number generator.
 *
 * Copyright 2012 Red Hat, Inc.
 * Copyright 2012 Amit Shah <amit.shah@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or
 * (at your option) any later version.  See the COPYING file in the
 * top-level directory.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/iov.h"
#include "qemu/module.h"
#include "qemu/timer.h"
#include "hw/virtio/virtio.h"
#include "hw/qdev-properties.h"
#include "tpm_virtio.h"

#include "sysemu/tpm_util.h"
#include "sysemu/runstate.h"
#include "sysemu/tpm.h"
#include "qom/object_interfaces.h"
#include "trace.h"

// hw/tpm/tpm_prop.h 无法 include

extern const PropertyInfo qdev_prop_tpm;

#define DEFINE_PROP_TPMBE(_n, _s, _f)                     \
    DEFINE_PROP(_n, _s, _f, qdev_prop_tpm, TPMBackend *)


// static bool is_guest_ready(VirtIORNG *vrng)
// {
//     VirtIODevice *vdev = VIRTIO_DEVICE(vrng);
//     if (virtio_queue_ready(vrng->vq)
//         && (vdev->status & VIRTIO_CONFIG_S_DRIVER_OK)) {
//         return true;
//     }
//     trace_virtio_rng_guest_not_ready(vrng);
//     return false;
// }

// static size_t get_request_size(VirtQueue *vq, unsigned quota)
// {
//     unsigned int in, out;

//     virtqueue_get_avail_bytes(vq, &in, &out, quota, 0);
//     return in;
// }


static int tpm_backend_worker_thread(gpointer data)
{
    TPMBackend *s = TPM_BACKEND(data);
    TPMBackendClass *k = TPM_BACKEND_GET_CLASS(s);
    Error *err = NULL;
    //printf("virtio_tpm before send %d",s->cmd->out_len);
    k->handle_request(s, s->cmd, &err);
    //printf("virtio_tpm after send %d",s->cmd->out_len);
    if (err) {
        error_report_err(err);
        return -1;
    }

    return 0;
}


static enum TPMVersion tpm_virtio_get_version(TPMIf *ti)
{
    //VirtIOTPM *vtpm = VirtIO_TPM(ti);

    return TPM_VERSION_2_0;
}


/*
 * Callback from the TPM to indicate that the response was received.
 */
static void tpm_virtio_request_completed(TPMIf *ti, int ret)
{
    // uint8_t locty = s->cmd.locty;
    // uint8_t l;

    // assert(TPM_TIS_IS_VALID_LOCTY(locty));

    // if (s->cmd.selftest_done) {
    //     for (l = 0; l < TPM_TIS_NUM_LOCALITIES; l++) {
    //         s->loc[l].sts |= TPM_TIS_STS_SELFTEST_DONE;
    //     }
    // }

    // /* FIXME: report error if ret != 0 */
    // tpm_tis_sts_set(&s->loc[locty],
    //                 TPM_TIS_STS_VALID | TPM_TIS_STS_DATA_AVAILABLE);
    // s->loc[locty].state = TPM_TIS_STATE_COMPLETION;
    // s->rw_offset = 0;

    // tpm_util_show_buffer(s->buffer, s->be_buffer_size, "From TPM");

    // if (TPM_TIS_IS_VALID_LOCTY(s->next_locty)) {
    //     tpm_tis_abort(s);
    // }

    // tpm_tis_raise_irq(s, locty,
    //                   TPM_TIS_INT_DATA_AVAILABLE | TPM_TIS_INT_STS_VALID);
}


static uint64_t get_features(VirtIODevice *vdev, uint64_t f, Error **errp)
{
    return f;
}

static void tpm_virtio_set_status(VirtIODevice *vdev, uint8_t status)
{
    // VirtIOTPM *vtpm = TPM_VIRTIO(vdev);

    if (!vdev->vm_running) {
        return;
    }
    vdev->status = status;

}

// static void tpm_virtio_init(Object *obj)
// {
//     VirtIOTPM *vtpm = TPM_VIRTIO(obj);

//     // device_add_bootindex_property(obj, &n->nic_conf.bootindex,
//     //                               "bootindex", "/ethernet-phy@0",
//     //                               DEVICE(n));
// }





static void handle_command(VirtIODevice *vdev, VirtQueue *vq, VirtQueueElement *elem){
    VirtIOTPM *vtpm = TPM_VIRTIO(vdev);

    //printf("out_buffer len is %d  and in_buffer len is %d  \n",elem->out_num,elem->in_num);
    //assert(elem->in_num == 1 && elem->out_num == 1);
    uint32_t buf_len = elem->out_sg[0].iov_len;
    uint8_t *buf = elem->out_sg[0].iov_base;
    


    
    //const uint8_t *out = buf;
    //uint32_t in_len = out_len;
    uint8_t *response = g_malloc(512*sizeof(uint8_t));
    uint32_t response_len = 512;
    //bool selftest_done =  true;
    struct TPMBackendCmd *cmd = g_malloc(sizeof(TPMBackendCmd));
    *cmd = (TPMBackendCmd){
        .in = buf,
        .in_len = buf_len,
        .out = response,
        .out_len = response_len,
        .selftest_done = true
    };
    //printf("\nQEMU virtio_tpm Response address: %p \n", cmd->out);
    //tpm_backend_deliver_request(vtpm->tpm, cmd);
    vtpm->tpm->cmd = cmd;
    tpm_backend_worker_thread(vtpm->tpm);

    
    //printf("\nReceive one message: %u \n", cmd->out_len);
    // for (size_t i = 0; i < cmd->out_len; ++i) {
    //     printf("%02x", cmd->out[i]);
    // }
    //printf("\n");
    //printf("\nQEMU : virtio_tpm: guest len is %lu and buffer len is %d\n", elem->in_sg[0].iov_len ,  cmd->out_len);
    assert( cmd->out_len <= elem->in_sg[0].iov_len);

    memcpy(elem->in_sg->iov_base,  cmd->out, cmd->out_len);
    virtqueue_push(vq,  elem, cmd->out_len);
    virtio_notify(VIRTIO_DEVICE(vdev),vq);
}
static void tpm_virtio_handle(VirtIODevice *vdev, VirtQueue *vq)
{

    // VirtIOTPM *vtpm = TPM_VIRTIO(dev);
    //ssize_t len, offset=0;
    VirtQueueElement *elem;
    //TPMBackendCmd cmd;
    //uint8_t *buf = NULL;

    if(!virtio_queue_ready(vq)){
        return;
    }

    //printf("qemu:virtio_tpm ready  pop elem\n");

    elem = virtqueue_pop(vq,sizeof(VirtQueueElement));
    while (elem)
    {
        handle_command(vdev, vq, elem);
        elem = virtqueue_pop(vq,sizeof(VirtQueueElement));
    }
    




    // elem = virtqueue_pop(vq, sizeof(VirtQueueElement));
    // // TODO: 读取完？
    // len = iov_from_buf(elem->in_sg, elem->in_num, 0,
    //                     buf, 6);
    // offset += len;

    //virtio_error(vdev, "lalalal %x %x %ld", buf[0], buf[1] , offset);

// // static void tpm_tis_tpm_send(TPMState *s, uint8_t locty)

//     tpm_util_show_buffer(s->buffer, s->be_buffer_size, "To TPM");

//     /*
//      * rw_offset serves as length indicator for length of data;
//      * it's reset when the response comes back
//      */
//     s->loc[locty].state = TPM_TIS_STATE_EXECUTION;

//     s->cmd = (TPMBackendCmd) {
//         .locty = locty,
//         .in = s->buffer,
//         .in_len = s->rw_offset,
//         .out = s->buffer,
//         .out_len = s->be_buffer_size,
//     };

    

    // tpm_backend_deliver_request(vtpm->tpm, &s->cmd);

}

static void tpm_virtio_device_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOTPM *vtpm = TPM_VIRTIO(dev);

    virtio_init(vdev, VIRTIO_ID_TPM, sizeof(TPMState));

    //vtpm->conf.tpm = vtpm->tpm;
    vtpm->state.be_driver = vtpm->tpm;
    vtpm->request_vq = virtio_add_queue(vdev, 64, tpm_virtio_handle);
    //vtpm->response_vq = virtio_add_queue(vdev, 64, tpm_virtio_idle);

    if (!tpm_find()) {
        error_setg(errp, "at most one TPM device is permitted");
        return;
    }

    if (!vtpm->tpm) {
        error_setg(errp, "'tpmdev' property is required");
        return;
    }

}

static void tpm_virtio_device_unrealize(DeviceState *dev)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    // VirtIOTPM *vtpm = TPM_VIRTIO(dev);

    virtio_del_queue(vdev, 0);


    virtio_cleanup(vdev);
}

// static const VMStateDescription vmstate_tpm_virtio = {
//     .name = "tpm-virtio",
//     .minimum_version_id = 1,
//     .version_id = 1,
//     .fields = (VMStateField[]) {
//         VMSTATE_VIRTIO_DEVICE,
//         VMSTATE_END_OF_LIST()
//     },
// };


static void tpm_virtio_reset(DeviceState *dev)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOTPM *vtpm = TPM_VIRTIO(vdev);
    TPMState *s = &vtpm->state;
    //int c;

    s->be_tpm_version = tpm_backend_get_tpm_version(s->be_driver);
    s->be_buffer_size = MIN(tpm_backend_get_buffer_size(s->be_driver),
                            TPM_TIS_BUFFER_MAX);

    if (s->ppi_enabled) {
        tpm_ppi_reset(&s->ppi);
    }
    tpm_backend_reset(s->be_driver);

    s->active_locty = TPM_TIS_NO_LOCALITY;
    s->next_locty = TPM_TIS_NO_LOCALITY;
    s->aborting_locty = TPM_TIS_NO_LOCALITY;

    // for (c = 0; c < TPM_TIS_NUM_LOCALITIES; c++) {
    //     s->loc[c].access = TPM_TIS_ACCESS_TPM_REG_VALID_STS;
    //     switch (s->be_tpm_version) {
    //     case TPM_VERSION_UNSPEC:
    //         break;
    //     case TPM_VERSION_1_2:
    //         s->loc[c].sts = TPM_TIS_STS_TPM_FAMILY1_2;
    //         s->loc[c].iface_id = TPM_TIS_IFACE_ID_SUPPORTED_FLAGS1_3;
    //         break;
    //     case TPM_VERSION_2_0:
    //         s->loc[c].sts = TPM_TIS_STS_TPM_FAMILY2_0;
    //         s->loc[c].iface_id = TPM_TIS_IFACE_ID_SUPPORTED_FLAGS2_0;
    //         break;
    //     }
    //     s->loc[c].inte = TPM_TIS_INT_POLARITY_LOW_LEVEL;
    //     s->loc[c].ints = 0;
    //     s->loc[c].state = TPM_TIS_STATE_IDLE;

    //     s->rw_offset = 0;
    // }

    if (tpm_backend_startup_tpm(s->be_driver, s->be_buffer_size) < 0) {
        exit(1);
    }

    
}

static Property tpm_virtio_properties[] = {
    DEFINE_PROP_TPMBE("tpmdev", VirtIOTPM, tpm),
    DEFINE_PROP_BOOL("ppi", VirtIOTPM, ppi_enabled, false),
    DEFINE_PROP_END_OF_LIST(),
};


static void tpm_virtio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    TPMIfClass *tc = TPM_IF_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);

    device_class_set_props(dc, tpm_virtio_properties);
    // dc->vmsd = &vmstate_tpm_virtio;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = tpm_virtio_device_realize;
    vdc->unrealize = tpm_virtio_device_unrealize;
    vdc->get_features = get_features;
    vdc->set_status = tpm_virtio_set_status;

    tc->model = TPM_MODEL_TPM_VIRTIO;
    tc->get_version = tpm_virtio_get_version;
    tc->request_completed = tpm_virtio_request_completed;

    dc->reset = tpm_virtio_reset;
}

static const TypeInfo tpm_virtio_info = {
    .name = TYPE_TPM_VIRTIO,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOTPM),
    // .instance_init = tpm_virtio_init, // 注意 instance_init 和 realize 的区别。
    .class_init = tpm_virtio_class_init,
    .interfaces = (InterfaceInfo[]) {
        { TYPE_TPM_IF }, // backends/tpm/tpm_backend.c
        // { TYPE_ACPI_DEV_AML_IF }, // hw/acpi/acpi_interface.c
        { }
    }
};

static void virtio_register_types(void)
{
    type_register_static(&tpm_virtio_info);
}

type_init(virtio_register_types)
