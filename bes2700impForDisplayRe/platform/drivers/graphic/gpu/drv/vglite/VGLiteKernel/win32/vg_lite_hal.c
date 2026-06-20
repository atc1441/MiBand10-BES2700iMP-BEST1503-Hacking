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
*
*****************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include "cmodel.h"
#include <Windows.h>
#include "../vg_lite_kernel.h"

typedef struct _device
{
    uint32_t irq_event;
    HANDLE   semaphore;
} device_t;

device_t s_Device = {0, NULL};

#define GET_DEVICE() \
    device_t *device = &s_Device

void irq_handler(unsigned int id, void * data)
{
    if (id !=0) {
        device_t *device = (device_t *) data;
        device->irq_event |= id;
        ReleaseSemaphore(device->semaphore, 1, NULL);
    }
}

void vg_lite_hal_delay(uint32_t milliseconds)
{
}

void vg_lite_hal_barrier(void)
{
}

void vg_lite_hal_initialize(void)
{
    GET_DEVICE();
    // Initialize the C-Model with 32MB of memory.
    CModelConstructor(64 << 20);

    // Register irq callback.
    CModelRegisterIrq(irq_handler, device);

    // Create event semaphore.
    device->semaphore = CreateSemaphore(NULL, 0, 1, "GPU event");
}

void vg_lite_hal_deinitialize(void)
{
    GET_DEVICE();
    CloseHandle(device->semaphore);
    // Destroy the C-Model.
    CModelDestructor();
}

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, void ** logical, uint32_t * physical, void ** node)
{
    // Allocate memory from the emulator.
    cmMemoryInfo * ptr = CModelAllocate(size, 1);

    if (ptr != NULL)
    {
        // Return logical and physical addresses.
        *logical  = ptr->logical;
        *physical = ptr->physical;
        *node = ptr;
        return VG_LITE_SUCCESS;
    }
    return VG_LITE_OUT_OF_MEMORY;
}

void vg_lite_hal_free_contiguous(void * memory)
{
    // Free the emulator memory.
    CModelFree((cmMemoryInfo*)memory);
}

void vg_lite_hal_free_os_heap(void)
{
    /* TODO: Remove unfree node. */
}

void * vg_lite_hal_map(unsigned long size, void * logical, uint32_t physical, uint32_t * gpu)
{
    // Not supported in emulator.
    return NULL;
}

void vg_lite_hal_unmap(void * handle)
{
    // Not supported in emulator.
}

uint32_t vg_lite_hal_peek(uint32_t address)
{
    // Read a C-Model register.
    return CModelReadRegister(address);
}

void vg_lite_hal_poke(uint32_t address, uint32_t data)
{
    // Write a C-Model register.
    CModelWriteRegister(address, data);
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    /* Not supported in emulator. */
    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    /* Not supported in emulator. */
    return VG_LITE_NOT_SUPPORT;
}

int32_t vg_lite_hal_wait_interrupt(uint32_t timeout, uint32_t mask, uint32_t * value)
{
    DWORD result;
    GET_DEVICE();
    if (timeout == 0xFFFFFFFF)
    {
        // Wait for interrupt.
        result = WaitForSingleObject(device->semaphore, INFINITE); // sem_wait(s_CModel->m_Interrupt);
    }
    else 
    {
        for (;;)
        {
            // Check if interrupt has happened.
            //if (sem_trywait(s_CModel->m_Interrupt) == 0)
            if (WAIT_OBJECT_0 == WaitForSingleObject(device->semaphore, 0))
            {
                // Success.
                result = 1;
                break;
            }

            // Check for a valid timeout.
            if (timeout-- != 0)
            {
                // Wait for 1 ms.
                Sleep(1); //usleep(1000);
            }
            else
            {
                // Timeout.
                result = 0;
                break;
            }
        }
    }

    return result;
}
