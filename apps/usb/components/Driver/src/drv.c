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

#include <dma/dma.h>
#include <platsupport/timer.h>
#include <platsupport/plat/pit.h>
#include <usb/drivers/cdc.h>

void camkes_make_simple(simple_t *simple);

usb_t usb;
ps_io_ops_t io_ops;
ps_mutex_ops_t mutex_ops;
// pit_t *pit_timer;
// pstimer_t *tsc_timer; TODO: figure out what this is for

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
	// irq_acknowledge();
}

void pit_irq_handle(void)
{
	// timer_handle_irq(pit_timer, 2);
	// pit_irq_acknowledge();
}

static int dma_morecore(size_t min_size, int cached,
		struct dma_mem_descriptor *dma_desc)
{
	uintptr_t vaddr, paddr;

	min_size = ROUND_UP(min_size, PAGE_SIZE_4K);

	vaddr = (uintptr_t) (min_size, PAGE_SIZE_4K, false);
	assert(vaddr);

	paddr = (uintptr_t)camkes_dma_get_paddr((void*)vaddr);
	assert(paddr);

	dma_desc->vaddr = vaddr;
	dma_desc->paddr = paddr;
	dma_desc->cached = 0;
	dma_desc->size_bits = PAGE_BITS_4K;
	dma_desc->alloc_cookie = NULL;
	dma_desc->cookie = NULL;

	return 0;
}

void pre_init(void)
{
	int err;

	mutex_ops.mutex_new = mutex_init;
	mutex_ops.mutex_lock = mutex_lock;
	mutex_ops.mutex_unlock = mutex_unlock;
	mutex_ops.mutex_destroy = mutex_destroy;

	camkes_io_ops(&io_ops);

	dma_dmaman_init(dma_morecore, NULL, &io_ops.dma_manager);

#ifdef CONFIG_IOMMU
    /* Temporary hack to map the DMA memory into the iommu */
    int error;
    simple_t camkes_simple;
    camkes_make_simple(&camkes_simple);
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
    assert(allocman);
    error = allocman_add_simple_untypeds(allocman, &camkes_simple);
    allocman_make_vka(&vka, allocman);
    cspacepath_t iospace;
    error = vka_cspace_alloc_path(&vka, &iospace);
    if (error) {
        ZF_LOGF("Failed to allocate iospace slot");
    }
    int bus, dev, fun;
    // sscanf(pci_bdf, "%x:%x.%d", &bus, &dev, &fun); // we get the bdf some other way now....
    bus = 0;
    dev = 26;
    fun = 0;
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

	// err = pit_init(pit_timer, io_ops.io_port_ops);
	// assert(!err);
	// tsc_timer = tsc_get_timer(pit_timer);
}

