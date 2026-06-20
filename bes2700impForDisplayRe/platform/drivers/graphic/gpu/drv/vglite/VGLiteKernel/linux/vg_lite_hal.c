/****************************************************************************
 *
 *   The MIT License (MIT)
 *
 *   Copyright (c) 2014 - 2020 Vivante Corporation
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *   DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
 

#include "vg_lite_platform.h"
#include "../vg_lite_kernel.h"
#include "../../inc/vg_lite_hal.h"
#include "vg_lite_ioctl.h"
#include "../vg_lite_hw.h"
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/mm_types.h>
#include <linux/io.h>

MODULE_LICENSE("Dual MIT/GPL");

static int  vg_lite_init(void);
static void vg_lite_exit(void);

static  ulong    register_mem_base = 0x02204000;
module_param(register_mem_base, ulong, 0644);
static  uint    register_mem_size = 0x00004000;
module_param(register_mem_size, uint, 0644);
static  uint    irq_line     = 43;
module_param(irq_line, uint, 0644);
static  uint    contiguous_size = 0x02000000;
module_param(contiguous_size, uint, 0644);
static  uint    contiguous_base = 0x38000000;
module_param(contiguous_base, uint, 0644);

#define HEAP_NODE_USED  0xABBAF00D

#if KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE
#define VM_FLAGS (VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP)
#else
#define VM_FLAGS (VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_RESERVED)
#endif

#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
#define current_mm_mmap_sem current->mm->mmap_lock
#else
#define current_mm_mmap_sem current->mm->mmap_sem
#endif

#define GET_PAGE_COUNT(size, p_size) \
( \
((size) + (p_size) - 1) / p_size \
)

/* Struct definitions. */
struct heap_node {
    struct list_head list;
    u32 offset;
    unsigned long size;
    s32 status;
};

struct memory_heap {
    u32 free;
    struct list_head list;
};

struct mapped_memory {
    void *logical;
    u32 physical;
    int page_count;
    struct page **pages;
};

struct vg_lite_device {
    void *gpu;             /* Register memory base */
    struct page *pages;
    unsigned int order;
    void *virtual;
    u32 physical;
    u32 size;
    struct memory_heap heap;
    int irq_enabled;

    volatile u32 int_flags;

    wait_queue_head_t int_queue;
    void *device;
    int registered;
    int major;
    struct class *class;
    int created;
};

struct client_data {
    struct vg_lite_device *device;
    struct vm_area_struct *vm;
    void *contiguous_mapped;
};

/* Data and objects declarations. */
static int heap_size = 8 << 20;     /* Default heap size is 16MB. */
static int verbose;
static int cached;

static struct vg_lite_device *device;
static struct client_data *private_data;

void vg_lite_hal_delay(u32 milliseconds)
{
    /* Delay the requested amount. */
    msleep(milliseconds);
}

void vg_lite_hal_barrier(void)
{
    /* Memory barrier. */
    smp_mb();

    /*Test for cached memory. */
    if (cached) {
        /* Flush the caches. */
#if KERNEL_VERSION(4, 19, 0) > LINUX_VERSION_CODE
        flush_cache_all();
#elif defined(__arm64__) || defined(__aarch64__)
        __flush_icache_all();
#endif
    }
}

void vg_lite_hal_initialize(void)
{
    /* TODO: Turn on the power. */

    /* TODO: Turn on the clock. */
}

void vg_lite_hal_deinitialize(void)
{
    /* TODO: Remove clock. */

    /* TODO: Remove power. */
}

static int split_node(struct heap_node *node, unsigned long size)
{
    struct heap_node *split;

    /* Allocate a new node. */
    split = kmalloc(sizeof(*split), GFP_KERNEL);
    if (!split)
        return -1;

    /* Fill in the data of this node of the remaning size. */
    split->offset = node->offset + size;
    split->size = node->size - size;
    split->status = 0;

    /* Add the new node behind the current node. */
    list_add(&split->list, &node->list);

    /* Adjust the size of the current node. */
    node->size = size;

    /* No error. */
    return 0;
}

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, void **logical, u32 *physical, void **node)
{
    unsigned long aligned_size;
    struct heap_node *pos;

    /* Align the size to 64 bytes. */
    aligned_size = VG_LITE_ALIGN(size, VGLITE_MEM_ALIGNMENT);

    /* Check if there is enough free memory available. */
    if (aligned_size > device->heap.free)
        return VG_LITE_OUT_OF_MEMORY;

    /* Walk the heap backwards. */
    list_for_each_entry_reverse(pos, &device->heap.list, list) {
        /* Check if the current node is free and is big enough. */
        if (pos->status == 0 && pos->size >= aligned_size) {
            /* See if we the current node is big enough to split. */
            if (pos->size - aligned_size >= VGLITE_MEM_ALIGNMENT) {
                if (split_node(pos, aligned_size))
                    return VG_LITE_OUT_OF_RESOURCES;
            }

            /* Mark the current node as used. */
            pos->status = HEAP_NODE_USED;

            /* Return the logical/physical address. */
            *logical = (uint8_t *)private_data->contiguous_mapped + pos->offset;
            *physical = device->physical + pos->offset;

            /* Update the heap free size. */
            device->heap.free -= aligned_size;

            *node = pos;
            return VG_LITE_SUCCESS;
        }
    }

    /*Out of memory. */
    return VG_LITE_OUT_OF_MEMORY;
}

void vg_lite_hal_free_contiguous(void *memory_handle)
{
    struct heap_node *pos, *node;

    /* Get pointer to node. */
    node = memory_handle;

    if (node->status != HEAP_NODE_USED) {
        if (verbose)
            printk(KERN_WARNING  "vg_lite: ignoring corrupted memory handle %p\n", memory_handle);
        return;
    }

    /* Mark node as free. */
    node->status = 0;

    /*Add node size to free count. */
    device->heap.free += node->size;

    /* Check if next node is free. */
    pos = node;
    list_for_each_entry_continue(pos, &device->heap.list, list) {
        if (!pos->status) {
            /*Merge the nodes. */
            node->size += pos->size;

            /*Delete the next node from the list. */
            list_del(&pos->list);
            kfree(pos);
        }
        break;
    }

    /* Check if the previous node is free. */
    pos = node;
    list_for_each_entry_continue_reverse(pos, &device->heap.list, list) {
        if (!pos->status) {
            /*Merge the nodes. */
            pos->size += node->size;

            /*Delete the current node from the list. */
            list_del(&node->list);
            kfree(node);
        }
        break;
    }
    /* TODO:the memory manager still have problem,we will refine it later.*/
    /* if(node->list.next == &device->heap.list && node->list.prev == &device->heap.list) */
    /*   kfree(node);*/
}

void vg_lite_hal_free_os_heap(void)
{
    /* TODO: Remove unfree node. */
}

u32 vg_lite_hal_peek(u32 address)
{
    /* Read data from the GPU register. */
    return *(u32 *)(uint8_t *)(device->gpu + address);
}

void vg_lite_hal_poke(u32 address, u32 data)
{
    /* Write data to the GPU register. */
    *(u32 *)(uint8_t *)(device->gpu + address) = data;
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    if (device) {
        mem->bytes = device->heap.free;
        return VG_LITE_SUCCESS;
    }
    mem->bytes = 0;
    return VG_LITE_NO_CONTEXT;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    void *_logical = NULL;
    u64 physical = node->physical;
    u32 offset = physical & (PAGE_SIZE - 1);
    u64 bytes = node->bytes + offset;
    u32 num_pages, pfn = 0;
    vg_lite_kernel_unmap_memory_t unmap_node;

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
    _logical = (void *)vm_mmap(NULL, 0L, bytes, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, 0);
#else
    down_write(&current_mm_mmap_sem);
    _logical = (void *)do_mmap_pgoff(NULL, 0L, bytes,
                PROT_READ | PROT_WRITE, MAP_SHARED, 0);
    up_write(&current_mm_mmap_sem);
#endif

    if (!_logical) {
        node->logical = NULL;
        return VG_LITE_OUT_OF_MEMORY;
    }

    down_write(&current_mm_mmap_sem);

    struct vm_area_struct *vma = find_vma(current->mm, (unsigned long)_logical);

    if (!vma)
        return VG_LITE_OUT_OF_RESOURCES;

    pfn = (physical >> PAGE_SHIFT);
    num_pages = GET_PAGE_COUNT(bytes, PAGE_SIZE);

    /* Make this mapping non-cached. */
    vma->vm_flags |= VM_FLAGS;

    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    if (remap_pfn_range(vma, vma->vm_start, pfn, num_pages << PAGE_SHIFT, vma->vm_page_prot) < 0)
        error = VG_LITE_OUT_OF_MEMORY;

    node->logical = (void *)((uint8_t *)_logical + offset);

    up_write(&current_mm_mmap_sem);

    if (error) {
        unmap_node.bytes = node->bytes;
        unmap_node.logical = node->logical;
        vg_lite_hal_unmap_memory(&unmap_node);
    }

    return error;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    void *_logical;
    u32 bytes;
    u32 offset = (u64)node->logical & (PAGE_SIZE - 1);

    if (unlikely(!current->mm))
        return error;

    _logical = (void *)((uint8_t *)node->logical - offset);
    bytes = GET_PAGE_COUNT(node->bytes + offset, PAGE_SIZE) * PAGE_SIZE;

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
    if (vm_munmap((unsigned long)_logical, bytes) < 0) {
        error = VG_LITE_INVALID_ARGUMENT;
        printk(KERN_WARNING "%s: vm_munmap failed\n", __func__);
    }
#else
    down_write(&current_mm_mmap_sem);
    if (do_munmap(current->mm, (unsigned long)_logical, bytes) < 0) {
       error = VG_LITE_INVALID_ARGUMENT;
       printk(KERN_WARNING "%s: do_munmap failed\n", __func__);
    }
    up_write(&current_mm_mmap_sem);
#endif

    return error;
}

s32 vg_lite_hal_wait_interrupt(u32 timeout, u32 mask, u32 *value)
{
    struct timeval tv;
    unsigned long jiffies;
    unsigned long result;

    if (timeout == VG_LITE_INFINITE) {
        /* Set 1 second timeout. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;
    } else {
        /* Convert timeout in ms to timeval. */
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    /* Convert timeval to jiffies. */
    jiffies = timeval_to_jiffies(&tv);

    /* Wait for interrupt, ignoring timeout. */
    do {
        result = wait_event_interruptible_timeout(device->int_queue, device->int_flags & mask, jiffies);
    } while (timeout == VG_LITE_INFINITE && result == 0);

    /* Report the event(s) got. */
    if (value)
        *value = device->int_flags & mask;

    device->int_flags = 0;
    return (result != 0);
}

void *vg_lite_hal_map(unsigned long bytes, void *logical, u32 physical, u32 *gpu)
{
    struct mapped_memory *mapped;
    int page_count;
    unsigned long start, end;

    if (logical) {
        start = (unsigned long)logical >> PAGE_SHIFT;
        end = ((unsigned long)logical + bytes + PAGE_SIZE - 1) >> PAGE_SHIFT;
        page_count = end - start;
    } else {
        page_count = 0;
    }

    mapped = kmalloc(sizeof(*mapped) + page_count * sizeof(struct page *), GFP_KERNEL);
    if (!mapped)
        return NULL;

    mapped->logical = logical;
    mapped->physical = physical;
    mapped->page_count = page_count;
    mapped->pages = page_count ? (struct page **)(mapped + 1) : NULL;

    if (!logical) {
        *gpu = physical;
    } else {
        down_read(&current->mm->mmap_sem);
#if KERNEL_VERSION(4, 6, 0) > LINUX_VERSION_CODE
        get_user_pages(current,
                       current->mm,
                       (unsigned long)logical & PAGE_MASK,
#else
        get_user_pages((unsigned long)logical & PAGE_MASK,
#endif
                       page_count,
#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
                       FOLL_WRITE,
#else
                       1,
                       0,
#endif
                       mapped->pages,
                       NULL);

        up_read(&current->mm->mmap_sem);
    }

    return mapped;
}

void vg_lite_hal_unmap(void *handle)
{
    struct mapped_memory *mapped = handle;
    int i;

    if (mapped->page_count) {
        for (i = 0; i < mapped->page_count; i++) {
            if (mapped->pages[i])
                put_page(mapped->pages[i]);
        }
    }

    kfree(mapped);
}

int drv_open(struct inode *inode, struct file *file)
{
    struct client_data *data;

    data = kmalloc(sizeof(*data), GFP_KERNEL);
    if (!data)
        return -1;

    data->device = device;
    data->contiguous_mapped = NULL;

    file->private_data = data;

    return 0;
}

int drv_release(struct inode *inode, struct file *file)
{
    struct client_data *data = (struct client_data *)file->private_data;

    kfree(data);
    file->private_data = NULL;

    return 0;
}

#ifdef HAVE_UNLOCKED_IOCTL
long drv_ioctl(struct file *file, unsigned int ioctl_code, unsigned long arg)
#else
long drv_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_code, unsigned long arg)
#endif
{
    struct ioctl_data arguments;
    void *data;

#ifndef HAVE_UNLOCKED_IOCTL
    /* inode will be not used */
    (void)inode;
#endif
    private_data = (struct client_data *)file->private_data;
    if (!private_data)
        return -1;

    if (ioctl_code != VG_LITE_IOCTL)
        return -1;

    if ((void *)!arg)
        return -1;

    if (copy_from_user(&arguments, (void *)arg, sizeof(arguments)) != 0)
        return -1;

    data = kmalloc(arguments.bytes, GFP_KERNEL);
    if (!data)
        return -1;

    if (copy_from_user(data, arguments.buffer, arguments.bytes) != 0)
        goto error;

    arguments.error = vg_lite_kernel(arguments.command, data);

    if (copy_to_user(arguments.buffer, data, arguments.bytes) != 0)
        goto error;

    kfree(data);

    if (copy_to_user((void *)arg, &arguments, sizeof(arguments)) != 0)
        return -1;

    return 0;

error:
    kfree(data);
    return -1;
}

ssize_t drv_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    struct client_data *private = (struct client_data *)file->private_data;

    if (length != 4)
        return 0;

    if (copy_to_user((void __user *)buffer, (const void *)&private->device->size, sizeof(private->device->size)) != 0)
        return 0;

    memcpy(buffer, &private->device->size, 4);
    return 4;
}

int drv_mmap(struct file *file, struct vm_area_struct *vm)
{
    unsigned long size;
    struct client_data *private = (struct client_data *)file->private_data;

    if (!cached)
#if defined(__arm64__) || defined(__aarch64__)
        vm->vm_page_prot = pgprot_writecombine(vm->vm_page_prot);
#else
        vm->vm_page_prot = pgprot_noncached(vm->vm_page_prot);
#endif

#if KERNEL_VERSION(3, 7, 0) > LINUX_VERSION_CODE
    vm->vm_flags |= VM_RESERVED;
#else
    vm->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
#endif
    vm->vm_pgoff = 0;

    size = vm->vm_end - vm->vm_start;
    if (size > private->device->size)
        size = private->device->size;

    if (remap_pfn_range(vm, vm->vm_start, private->device->physical >> PAGE_SHIFT, size, vm->vm_page_prot) < 0) {
        printk(KERN_WARNING "vg_lite: remap_pfn_range failed\n");
        return -1;
    }

    private->vm = vm;
    private->contiguous_mapped = (void *)vm->vm_start;

    if (verbose)
        printk(KERN_WARNING "vg_lite: mapped %scached contiguous memory to %p\n", cached ? "" : "non-", private->contiguous_mapped);

    return 0;
}

static const struct file_operations file_operations = {
    .owner          = THIS_MODULE,
    .open           = drv_open,
    .release        = drv_release,
    .read           = drv_read,
#ifdef HAVE_UNLOCKED_IOCTL
    .unlocked_ioctl = drv_ioctl,
#endif
#ifdef HAVE_COMPAT_IOCTL
    .compat_ioctl   = drv_ioctl,
#endif
    .mmap           = drv_mmap,
};

static void vg_lite_exit(void)
{
    struct heap_node *pos;
    struct heap_node *n;

    /* Check for valid device. */
    if (device) {
        if (device->gpu) {
            /* Unmap the GPU registers. */
            iounmap(device->gpu);
            device->gpu = NULL;
        }

        if (device->pages)
            /* Free the contiguous memory. */
            __free_pages(device->pages, device->order);

        if (device->irq_enabled)
            /* Free the IRQ. */
            free_irq(irq_line/*GPU_IRQ*/, device);

        /* Process each node. */
        list_for_each_entry_safe(pos, n, &device->heap.list, list) {
            /* Remove it from the linked list. */
            list_del(&pos->list);

            /* Free up the memory. */
            kfree(pos);
        }

        if (device->created)
            /* Destroy the device. */
            device_destroy(device->class, MKDEV(device->major, 0));

        if (device->class)
            /* Destroy the class. */
            class_destroy(device->class);

        if (device->registered)
            /* Unregister the device. */
            unregister_chrdev(device->major, "vg_lite");

        /* Free up the device structure. */
        kfree(device);
    }
}

static irqreturn_t irq_handler(int irq, void *context)
{
    struct vg_lite_device *device = context;

    /* Read interrupt status. */
    u32 flags = *(u32 *)(uint8_t *)(device->gpu + VG_LITE_INTR_STATUS);

    if (flags) {
        /* Combine with current interrupt flags. */
        device->int_flags |= flags;

        /* Wake up any waiters. */
        wake_up_interruptible(&device->int_queue);

        /* We handled the IRQ. */
        return IRQ_HANDLED;
    }

    /* Not our IRQ. */
    return IRQ_NONE;
}

static int vg_lite_init(void)
{
    struct heap_node *node;

    /* Create device structure. */
    device = kmalloc(sizeof(*device), GFP_KERNEL);
    if (!device) {
        printk(KERN_WARNING "vg_lite: kmalloc failed\n");
        return -1;
    }

    /* Zero out the enture structure. */
    memset(device, 0, sizeof(struct vg_lite_device));

    /* Map the GPU registers. */
    device->gpu = ioremap_nocache(register_mem_base/*GPU_REG_START*/, register_mem_size/*GPU_REG_SIZE*/);
    if (!device->gpu) {
        printk(KERN_WARNING "vg_lite: ioremap failed\n");
        return -1;
    }

    /* Allocate the contiguous memory. */
    device->virtual = 0;
    device->physical = contiguous_base;
    device->size = contiguous_size;
    if (verbose) {
        u32 size = (device->size >> 20) ? (device->size >> 20) : (device->size >> 10);
        char c = (device->size >> 20) ? 'M' : 'k';

        printk(KERN_WARNING "vg_lite: allocated a %u%cB heap at 0x%08x\n", size, c, device->physical);
    }

    /* Create the heap. */
    INIT_LIST_HEAD(&device->heap.list);
    device->heap.free = device->size;

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node) {
        printk(KERN_WARNING "vg_lite: kmalloc failed\n");
        vg_lite_exit();
        return -1;
    }
    node->offset = 0;
    node->size = device->size;
    node->status = 0;
    list_add(&node->list, &device->heap.list);

    /* Initialize the wait queue. */
    init_waitqueue_head(&device->int_queue);

    /* Install IRQ. */
    if (request_irq(irq_line/*GPU_IRQ*/, irq_handler, 0, "vg_lite_irq", device)) {
        printk(KERN_WARNING "vg_lite: request_irq failed\n");
        vg_lite_exit();
        return -1;
    }
    device->irq_enabled = 1;
    if (verbose)
        printk(KERN_WARNING "vg_lite: enabled ISR for interrupt %d\n", irq_line/*GPU_IRQ*/);

    /* Register device. */
    device->major = register_chrdev(0, "vg_lite", &file_operations);
    if (device->major < 0) {
        printk(KERN_WARNING "vg_lite: register_chrdev failed\n");
        vg_lite_exit();
        return -1;
    }
    device->registered = 1;

    /* Create the graphics class. */
    device->class = class_create(THIS_MODULE, "vg_lite_class");
    if (!device->class) {
        printk(KERN_WARNING "vg_lite: class_create failed\n");
        vg_lite_exit();
        return -1;
    }

    /* Create the device. */
    if (!device_create(device->class, NULL, MKDEV(device->major, 0), NULL, "vg_lite")) {
        printk(KERN_WARNING "vg_lite: device_create failed\n");
        vg_lite_exit();
        return -1;
    }
    device->created = 1;

    if (verbose)
        printk(KERN_WARNING "vg_lite: created /dev/vg_lite device\n");

    /* Success. */
    return 0;
}

module_param(heap_size, int, 0444);
module_param(verbose, int, 0444);
module_param(cached, int, 0444);
module_init(vg_lite_init)
module_exit(vg_lite_exit)
