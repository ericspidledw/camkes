/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the term of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(D61_BSD)
 */

#include <camkes.h>
#include <camkes/io.h>
// #include <camkes/dma.h>
#include <stdio.h>
#include <stdlib.h>

#include <usb/usb.h>
#include <simple/simple.h>
#include <allocman/bootstrap.h>
#include <allocman/allocman.h>
#include <allocman/vka.h>
#include <sel4utils/mapping.h>
#include <vka/capops.h>
#include <simple/simple_helpers.h>

#include <camkes.h>
#include <camkes/io.h>
#include <camkes/dma.h>
#include <stdio.h>
#include <stdlib.h>
#include <simple/simple_helpers.h>

#include <usb/plat/usb.h>
#include <usb.h>
#include <simple/simple.h>
#include <allocman/bootstrap.h>
#include <allocman/allocman.h>
#include <allocman/vka.h>
#include <sel4utils/mapping.h>
#include <vka/capops.h>

#include <platsupport/timer.h>
#include <platsupport/plat/pit.h>
#include <usb/drivers/cdc.h>

usb_t usb;
ps_io_ops_t io_ops;
ps_mutex_ops_t mutex_ops;

static void *mutex_init(void)
{
	return (void *)1234;
}

static int mutex_lock(void *mutex)
{
	return m_lock();
}

static int mutex_unlock(void *mutex)
{
	return m_unlock();
}

static int mutex_destroy(void *mutex)
{
	return 0;
}

void irq_handle(void)
{
	usb_handle_irq(&usb);
	irq_acknowledge();
}

static void* (*dma_alloc)(void *cookie, size_t size, int align, int cached,
		ps_mem_flags_t flags);
static void (*dma_free)(void *cookie, void *addr, size_t size);

static void* dma_alloc_hack(void *cookie, size_t size, int align, int cached,
		ps_mem_flags_t flags)
{
	void *p = NULL;
    int ret = 0;
	ret = dma_lock();
    assert(!ret);
	if (dma_alloc) {
		p = dma_alloc(cookie, size, align, cached, flags);
	}
	ret = dma_unlock();
    assert(!ret);

	return p;
}

static void dma_free_hack(void *cookie, void *addr, size_t size)
{
	int ret =dma_lock();
    assert(!ret);
	if (dma_free) {
		dma_free(cookie, addr, size);
	}
	ret = dma_unlock();
    assert(!ret);
}

void pre_init(void)
{
	int err;
    printf("We're in pre init\n");
	mutex_ops.mutex_new = mutex_init;
	mutex_ops.mutex_lock = mutex_lock;
	mutex_ops.mutex_unlock = mutex_unlock;
	mutex_ops.mutex_destroy = mutex_destroy;

	camkes_io_ops(&io_ops);
    printf("established our camkes io ops\n");

	dma_alloc = io_ops.dma_manager.dma_alloc_fn;
	dma_free = io_ops.dma_manager.dma_free_fn;

	io_ops.dma_manager.dma_alloc_fn = dma_alloc_hack;
	io_ops.dma_manager.dma_free_fn = dma_free_hack;

#ifdef CONFIG_IOMMU
    /* Temporary hack to map the DMA memory into the iommu */
    int error;
    simple_t camkes_simple;
    camkes_make_simple(&camkes_simple);
    printf("made our simple\n");
    allocman_t *allocman;
    vka_t vka;
    static char allocator_mempool[65536];
    /* Initialize allocator */
    allocman = bootstrap_use_current_1level(
            simple_get_cnode(&camkes_simple),
            simple_get_cnode_size_bits(&camkes_simple),
            simple_last_valid_cap(&camkes_simple) + 1,
            BIT(simple_get_cnode_size_bits(&camkes_simple)),
            sizeof(allocator_mempool), allocator_mempool
    );
    printf("Allocman created using boostrap\n");
    assert(allocman);
    error = allocman_add_simple_untypeds(allocman, &camkes_simple);
    allocman_make_vka(&vka, allocman);
    cspacepath_t iospace;
    error = vka_cspace_alloc_path(&vka, &iospace);
    if (error) {
        ZF_LOGF("Failed to allocate iospace slot");
    }
    int bus, dev, fun;
    sscanf(pci_bdf, "%x:%x.%d", &bus, &dev, &fun);
    int pci_bdf_int =  bus * 256 + dev * 8 + fun;
    error = simple_get_iospace(&camkes_simple, iospace_id, pci_bdf_int, &iospace);
    if (error) {
        ZF_LOGF("Failed to get iospace");
    }
    int total_bufs = 1024;
    int num_bufs = 0;
    void **dma_bufs = malloc(sizeof(void*) * total_bufs);
    if (!dma_bufs) {
        ZF_LOGF("Failed to malloc");
    }
    while (1) {
        if (num_bufs + 1 == total_bufs) {
            total_bufs *= 2;
            dma_bufs = realloc(dma_bufs, sizeof(void*) * total_bufs);
            if (!dma_bufs) {
                ZF_LOGF("Failed to realloc");
            }
        }
        void *buf = camkes_dma_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K, false);
        if (!buf) {
            break;
        }
        dma_bufs[num_bufs] = buf;
        num_bufs++;
        seL4_CPtr frame = camkes_dma_get_cptr(buf);
        cspacepath_t frame_path;
        uintptr_t paddr = camkes_dma_get_paddr(buf);
        cspacepath_t frame_dup;
        vka_cspace_make_path(&vka, frame, &frame_path);
        error = vka_cspace_alloc_path(&vka, &frame_dup);
        if (error) {
            ZF_LOGF("Failed to allocate frame cap");
        }
        error = vka_cnode_copy(&frame_dup, &frame_path, seL4_AllRights);
        if (error) {
            ZF_LOGF("Faileed to copy frame");
        }
        vka_object_t pts[5];
        int num_pts = 5;
        error = sel4utils_map_iospace_page(&vka, iospace.capPtr, frame_dup.capPtr, paddr, seL4_AllRights, 1, PAGE_BITS_4K, pts, &num_pts);
        if (error) {
            ZF_LOGF("Failed to map iospace page");
        }
    }
    for (int i = 0; i < num_bufs; i++) {
        camkes_dma_free(dma_bufs[i], PAGE_SIZE_4K);
    }
    free(dma_bufs);
#endif /* CONFIG_IOMMU */

	err = usb_init(USB_HOST_DEFAULT, &io_ops, &mutex_ops, &usb);
	assert(!err);
}

