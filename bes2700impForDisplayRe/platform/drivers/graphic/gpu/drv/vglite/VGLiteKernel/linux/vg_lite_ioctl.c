/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2020 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*

*****************************************************************************/

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "vg_lite_kernel.h"
#include "vg_lite_ioctl.h"

static int device;
static unsigned int length;
static void *mapped;

vg_lite_error_t vg_lite_kernel(vg_lite_kernel_command_t command, void *data)
{
    struct ioctl_data to_kernel;
    static const unsigned int bytes[] = {
        sizeof(vg_lite_kernel_initialize_t),
        sizeof(vg_lite_kernel_terminate_t),
        sizeof(vg_lite_kernel_allocate_t),
        sizeof(vg_lite_kernel_free_t),
        sizeof(vg_lite_kernel_submit_t),
        sizeof(vg_lite_kernel_wait_t),
        sizeof(vg_lite_kernel_reset_t),
        sizeof(vg_lite_kernel_debug_t),
        sizeof(vg_lite_kernel_map_t),
        sizeof(vg_lite_kernel_unmap_t),
        sizeof(vg_lite_kernel_info_t),
        sizeof(vg_lite_kernel_mem_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_flexa_info_t),
        sizeof(vg_lite_kernel_map_memory_t),
        sizeof(vg_lite_kernel_unmap_memory_t),
    };

    if (device == 0) {
        device = open("/dev/vg_lite", O_RDWR);
        if (device == -1)
            return VG_LITE_GENERIC_IO;
        if (read(device, &length, 4) != 4) {
            close(device);
            device = 0;
            return VG_LITE_GENERIC_IO;
        }

        /* Default contiguous mapping size. */
        length = MAX_CONTIGUOUS_SIZE;

        mapped = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, device, 0);
        if (!mapped ||
            mapped == (char *)-1) {
            close(device);
            device = 0;
            return VG_LITE_GENERIC_IO;
        }
    }

    to_kernel.command = command;
    to_kernel.buffer = data;
    to_kernel.bytes = bytes[command];

    if (ioctl(device, VG_LITE_IOCTL, &to_kernel) < 0)
        return VG_LITE_GENERIC_IO;

    return to_kernel.error;
}
