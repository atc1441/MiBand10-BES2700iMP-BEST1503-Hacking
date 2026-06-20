#include <stdarg.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include <unistd.h>
#include "aqcm.hpp"
#include "cmodel.h"
#include <windows.h>

struct GC355 : public CModel
{
public:
    HANDLE      m_Semaphore;
    bool        m_Quit;
    HANDLE      m_Thread;
    HANDLE      m_Interrupt;
    U32         m_IRQ;

    // For irq notification with kernel code.
    IRQ_HANDLER m_IrqHandler;
    void *      m_Context;

    GC355() : CModel()
    {
        //LPSECURITY_ATTRIBUTES attrib;
        m_Semaphore = CreateSemaphore(NULL, 0, 1, "gc355_cmodel_semaphore"); //sem_open("gc355_cmodel_semaphore", O_CREAT, S_IRUSR | S_IWUSR, 0);
        if (m_Semaphore == INVALID_HANDLE_VALUE)// SEM_FAILED)
        {
            printf("gc355_cmodel_semaphore returned %d\n", errno);
        }
        m_Interrupt = CreateSemaphore(NULL, 0, 1, "gc355_interrupt_semaphore");// sem_open("gc355_interrupt_semaphore", O_CREAT, S_IRUSR | S_IWUSR, 0);
        if (m_Interrupt == INVALID_HANDLE_VALUE) // SEM_FAILED)
        {
            printf("gc355_interrupt_semaphore returned %d\n", errno);
        }
        m_Quit = false;

        m_IrqHandler = NULL;
        m_Context = NULL;
        m_IRQ = 0;
    }

    virtual ~GC355()
    {
        //sem_close(m_Semaphore);
        //sem_close(m_Interrupt);
    }
};

static GC355 * s_CModel;

#if 0 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark C-Model Memory Model
#endif

class EmulatorMemory : public aqMemoryBase
{
protected:
    size_t          m_PageCount;
    void **         m_PageMap;
    HANDLE /*pthread_mutex_t*/ m_Critical;

public:
    EmulatorMemory(size_t TotalMemory)
    {
        // Allocate the memory.
        m_PageCount = TotalMemory >> 12;
        m_PageMap   = new void * [m_PageCount];
        memset(m_PageMap, 0, sizeof(void *) * m_PageCount);

        // Initialize the mutex.
        m_Critical = CreateMutex(NULL, FALSE, "memory_mutex");// pthread_mutex_init(&m_Critical, NULL);
    }

    virtual ~EmulatorMemory(void)
    {
        // Free the memory.
        delete [] m_PageMap;

        // Destroy the mutex.
         //pthread_mutex_destroy(&m_Critical);
    }

    virtual U32 GetMemorySize(void)
    {
        // Return memory size.
        return static_cast<U32>(m_PageCount << 12);
    }

    virtual bool SetMemorySize(U32 MemorySize)
    {
        return false;
    }

    virtual U32 Read(U32 Address)
    {
        void * logical;
        U32 data = 0;

        // Get logical pointer.
        logical = GetLogical(Address);
        if (logical != NULL)
        {
            // Read data from the memory.
            data = *reinterpret_cast<U32 *>(logical);
        }

        // Return the data.
        return data;
    }

    virtual void Write(U32 Address, U04 Mask, U32 Data)
    {
        void * logical;

        // Get logical pointer.
        logical = GetLogical(Address);
        if (logical != NULL)
        {
            // Dispatch on mask.
            switch (Mask)
            {
                case 0x1:
                    // Lower 8-bit write.
                    reinterpret_cast<U08 *>(logical)[0] = static_cast<U08>(GETBITS(Data, 7, 0));
                    break;

                case 0x2:
                    // Mid-lower 8-bit write.
                    reinterpret_cast<U08 *>(logical)[1] = static_cast<U08>(GETBITS(Data, 15, 8));
                    break;

                case 0x4:
                    // Mid-upper 8-bit write.
                    reinterpret_cast<U08 *>(logical)[2] = static_cast<U08>(GETBITS(Data, 23, 16));
                    break;

                case 0x8:
                    // Upper 8-bit write.
                    reinterpret_cast<U08 *>(logical)[3] = static_cast<U08>(GETBITS(Data, 31, 24));
                    break;

                case 0x3:
                    // Lower 16-bit write.
                    reinterpret_cast<U16 *>(logical)[0] = static_cast<U16>(GETBITS(Data, 15, 0));
                    break;

                case 0xC:
                    // Upper 16-bit write.
                    reinterpret_cast<U16 *>(logical)[1] = static_cast<U16>(GETBITS(Data, 31, 16));
                    break;

                case 0xF:
                    // 32-bit write.
                    reinterpret_cast<U32 *>(logical)[0] = Data;
                    break;

                default:
                    if (Mask & 0x1)
                    {
                        // Write bits [7:0].
                        reinterpret_cast<U08 *>(logical)[0] = static_cast<U08>(GETBITS(Data, 7, 0));
                    }
                    if (Mask & 0x2)
                    {
                        // Write bits [15:8].
                        reinterpret_cast<U08 *>(logical)[1] = static_cast<U08>(GETBITS(Data, 15, 8));
                    }
                    if (Mask & 0x4)
                    {
                        // Write bits [23:16].
                        reinterpret_cast<U08 *>(logical)[2] = static_cast<U08>(GETBITS(Data, 23, 16));
                    }
                    if (Mask & 0x8)
                    {
                        // Write bits [31:24].
                        reinterpret_cast<U08 *>(logical)[3] = static_cast<U08>(GETBITS(Data, 31, 24));
                    }
                    break;
            }
        }
    }

    virtual void* GetMemoryAddress(void) const
    {
        return NULL;
    }

    void * GetLogical(U32 Physical)
    {
        // Convert physical address to page index.
        U32 page = Physical >> 12;
        if (page >= m_PageCount)
        {
            // Page is out-of-bounds.
            return NULL;
        }

        // Get logical address.
        void * logical = m_PageMap[page];
        if (logical == NULL)
        {
            // Page is not allocated.
            return NULL;
        }

        // Add page offset to logical address.
        return static_cast<U08 *>(logical) + (Physical & 0xFFF);
    }

    U32 GetPhysical(void * Logical)
    {
        // Cast the pointer to byte-addressable memory.
        U08 * address = static_cast<cmUInt8 *>(Logical);

        for (cmSize i = 0; i < m_PageCount; i++)
        {
            // Get mapped address.
            U08 * map = static_cast<U08 *>(m_PageMap[i]);

            // Check if the pointer is inside this mapped page.
            if ((address >= map) && (address < map + 4096))
            {
                // Return physical address for this pointer.
                return static_cast<U32>((i << 12) + (address - map));
            }
        }

        // Invalid address.
        return ~0U;
    }

    cmMemoryInfo * AllocateContiguous(size_t Bytes)
    {
        // Compute number of required pages.
        size_t pages = (Bytes + 4095) >> 12;

        // Create a new memory info structure.
        cmMemoryInfo *memory = new cmMemoryInfo;
        if (memory == NULL)
        {
            // Out of memory.
            return NULL;
        }

        // Allocate a new memory blob.
        U08 * logical = new U08 [pages << 12];
        if (logical == NULL)
        {
            // Delete the memory info structure.
            delete memory;

            // Out of memory.
            return NULL;
        }

        // Lock the mutex.
        WaitForSingleObject(m_Critical, INFINITE); //pthread_mutex_lock(&m_Critical);

        // Process all pages.
        for (size_t i = 0; i < m_PageCount - pages; i++)
        {
            // Check if there are enough free pages.
            size_t j;
            for (j = 0; j < pages; j++)
            {
                if (m_PageMap[i + j] != NULL)
                {
                    // Page is in use, break loop.
                    break;
                }
            }

            // Check if this range is large enough.
            if (j == pages)
            {
                // Process all pages in the range.
                for (j = 0; j < pages; j++)
                {
                    // Map the pages.
                    m_PageMap[i + j] = static_cast<void *>(logical + (j << 12));
                }

                // Unlock the mutex.
                ReleaseMutex(m_Critical);//pthread_mutex_unlock(&m_Critical);

                // Fill in the memory information.
                memory->physical = static_cast<U32>(i << 12);
                memory->logical  = logical;
                memory->bytes    = pages << 12;

                // Return the memory info structure.
                return memory;
            }
        }

        // Unlock the mutex.
        ReleaseMutex(m_Critical);//pthread_mutex_unlock(&m_Critical);

        // Delete the memory blob.
        delete [] logical;

        // Delete the memory info structure.
        delete memory;

        // Out of memory.
        return NULL;
    }

    cmMemoryInfo * Allocate(size_t Bytes)
    {
        // Compute the required number of pages.
        size_t pages = (Bytes + 4095) >> 12;

        // Create a new memory info structure.
        cmMemoryInfo * memory = new cmMemoryInfo;
        if (memory == NULL)
        {
            // Out of memory.
            return NULL;
        }

        // Allocate a new memory blob.
        U08 * logical = new U08 [pages << 12];
        if (logical == NULL)
        {
            // Delete the memory info structure.
            delete memory;

            // Out of memory.
            return NULL;
        }

        // Fill in the memory info structure.
        memory->logical = logical;
        memory->bytes   = pages << 12;

        // Lock the mutex.
        WaitForSingleObject(m_Critical, INFINITE);  // pthread_mutex_lock(&m_Critical);

        // Process all pages.
        size_t j = 0;
        for (size_t i = 0; (i < m_PageCount) && (pages > 0); i++)
        {
            // Test if page is empty.
            if (m_PageMap[i] == NULL)
            {
                // Test if this is the first page.
                if (j == 0)
                {
                    // Set physiucal memory.
                    memory->physical = U32(i << 12);
                }

                // Set localgical address for page.
                m_PageMap[i] = logical + j;

                // Next logical address.
                j += 1 << 12;

                // Decrease the number of pages.
                pages--;
            }
        }

        // Unlock the mutex.
        ReleaseMutex(m_Critical);// pthread_mutex_unlock(&m_Critical);

        // Test if we are out of memory.
        if (pages > 0)
        {
            if (j)
            {
                // Free the allocated memory.
                Free(memory);
            }
            else
            {
                // Delete loglcal memory.
                delete logical;

                // Delete the memory info structure.
                delete memory;
            }

            // Out of memory.
            return NULL;
        }

        // Return the memory info pointer.
        return memory;
    }

    void Free(cmMemoryInfo *Memory)
    {
        // Compute the number of pages and the initial page.
        size_t pages = Memory->bytes    >> 12;
        size_t page  = Memory->physical >> 12;

        // Set range of logical memory.
        U08 * start = static_cast<U08 *>(Memory->logical);
        U08 * end   = start + Memory->bytes;

        // Lock the mutex.
        WaitForSingleObject(m_Critical, INFINITE); // pthread_mutex_lock(&m_Critical);

        // Check if the starting page is valid.
        if (page < m_PageCount)
        {
            // Process all pages.
            for (size_t i = page; pages > 0;)
            {
                // Check if the current page is in the logical memory range.
                if (m_PageMap[i] >= start && m_PageMap[i] < end)
                {
                    // Mark page as empty.
                    m_PageMap[i] = NULL;

                    // Decrease the number of pages.
                    pages--;
                }

                // Increase the page index and test for overflow.
                if (++i >= m_PageCount)
                {
                    // Wrap to page 0.
                    i = 0;
                }
                else if ((i == page) && (pages > 0))
                {
                    // No more pages found.
                    break;
                }
            }
        }

        // Unlock the mutex.
        ReleaseMutex(m_Critical); //pthread_mutex_unlock(&m_Critical);

        // Delete the logical memory.
        delete Memory->logical;

        // Delete the memory info structure.
        delete Memory;
    }
};

#if 0 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark C-Model Callback Functions
#endif

static void CModelInterrupt(void * Context)
{
    // Get pointer to C-Model.
    GC355 * cmodel = static_cast<GC355 *>(Context);

    // Read the ISR.
    cmodel->m_IRQ |= CModelReadRegister(0x0010);

    // Set interrupt semaphore.
    ReleaseSemaphore(cmodel->m_Interrupt, 1, NULL);  // sem_post(cmodel->m_Interrupt);

    // Call kernel irq callback.
    if (cmodel->m_IrqHandler != NULL) {
        (*cmodel->m_IrqHandler)(cmodel->m_IRQ, cmodel->m_Context);
        cmodel->m_IRQ = 0;
    }
}

//static void * CModelThread(void * Context)
static DWORD WINAPI CModelThread(LPVOID Context)
{
    // Get pointer to C-Model.
    GC355 * cmodel = static_cast<GC355 *>(Context);

    // Loop forever.
    for (;;)
    {
        // Wait for a semaphore.
        WaitForSingleObject(cmodel->m_Semaphore, INFINITE); //sem_wait(cmodel->m_Semaphore);

        // Check if we are asked to quit.
        if (cmodel->m_Quit)
        {
            break;
        }

        // Call C-Model.
        AQPARAMETERS parameters;
        parameters.IsStateChange = FALSE;
        gcSTAGE_INSTANCE(FE)->DoStep(parameters);
    }

    // Terminate the C-Model thread.
    return NULL;
}

void CModelPrint(const char *Message, ...)
{
    // Print to stdout.
    va_list arguments;
    va_start(arguments, Message);
    vprintf(Message, arguments);
    va_end(arguments);
}

#if 0 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark C-Model API
#endif

int CModelConstructor(size_t Memory)
{
    // Create the memory.
    _SystemMemory = new EmulatorMemory(Memory);
    if (_SystemMemory == NULL)
    {
        // Out of memory.
        return -1;
    }

    // Build C-Model.
    s_CModel = new GC355();
    if (s_CModel == NULL)
    {
        // Delete the memory.
        delete _SystemMemory;

        // Out of memory.
        return -1;
    }

    // Set interrupt call-back function.
    gcSTAGE_INSTANCE(MC)->GetInterrupt().RegisterCallback(CModelInterrupt, s_CModel);

    // Create C-Model thread.
    s_CModel->m_Thread = CreateThread(NULL, 0, CModelThread, s_CModel, 0, NULL);// pthread_create(&s_CModel->m_Thread, NULL, CModelThread, s_CModel);

    // Success.
    return 0;
}

void CModelDestructor(void)
{
    // Destroy the C-Model thread.
    s_CModel->m_Quit = true;
    ReleaseSemaphore(s_CModel->m_Semaphore, 1, NULL);// sem_post(s_CModel->m_Semaphore);
    WaitForSingleObject(s_CModel->m_Thread, INFINITE);// pthread_join(s_CModel->m_Thread, NULL);

    // Destroy the C-Model.
    delete s_CModel;
    s_CModel = NULL;

    // Destroy the memory.
    delete _SystemMemory;
    _SystemMemory = NULL;
}

void CModelWriteRegister(cmUInt32 Address, cmUInt32 Data)
{
    // Write the register to the C-Model.
    AQPARAMETERS parameters;
    parameters.IsStateChange              = TRUE;
    parameters.u.aqState.IsRegisterAccess = TRUE;
    parameters.u.aqState.StateAddress     = Address >> 2;
    parameters.u.aqState.StateValue       = Data;
    gcSTAGE_INSTANCE(MC)->DoStep(parameters);

    // Test for special registers.
    if (Address == GCREG_FETCH_CONTROL_Address)
    {
        // Run the C-Model.
        ReleaseSemaphore(s_CModel->m_Semaphore, 1, NULL);// sem_post(s_CModel->m_Semaphore);
    }
}

cmUInt32 CModelReadRegister(cmUInt32 Address)
{
    // Read register from the C-Model.
    U32 data = 0;
    gcSTAGE_INSTANCE(MC)->ReadRegister(Address, data);
    return data;
}

cmMemoryInfo * CModelAllocate(size_t Bytes, cmBool Contiguous)
{
    // Allocate memory from the emulator.
    return (  Contiguous
            ? reinterpret_cast<EmulatorMemory *>(_SystemMemory)->AllocateContiguous(Bytes)
            : reinterpret_cast<EmulatorMemory *>(_SystemMemory)->Allocate(Bytes));
}

void CModelFree(cmMemoryInfo * Memory)
{
    // Free emualtor memory.
    reinterpret_cast<EmulatorMemory *>(_SystemMemory)->Free(Memory);
}

cmUInt32 CModelPhysical(void * Logical)
{
    // Get physical address for a logical pointer.
    return reinterpret_cast<EmulatorMemory *>(_SystemMemory)->GetPhysical(Logical);
}

cmBool CModelWaitInterrupt(cmUInt32 MillseSeconds)
{
    // Check for infinite timeout.
    if (MillseSeconds == 0xFFFFFFFF)
    {
        // Wait for interrupt.
        WaitForSingleObject(s_CModel->m_Interrupt, INFINITE); // sem_wait(s_CModel->m_Interrupt);

        // Success.
        return TRUE;
    }

    // Loop forever.
    for (;;)
    {
        // Check if interrupt has happened.
        //if (sem_trywait(s_CModel->m_Interrupt) == 0)
        if (WAIT_OBJECT_0 == WaitForSingleObject(s_CModel->m_Interrupt, 0))
        {
            // Success.
            return TRUE;
        }

        // Check for a valid timeout.
        if (MillseSeconds-- != 0)
        {
            // Wait for 1 ms.
            Sleep(1); //usleep(1000);
        }
        else
        {
            // Timeout.
            return FALSE;
        }
    }
}

cmBool CModelRegisterIrq(IRQ_HANDLER handler, void * private_data)
{
    s_CModel->m_IrqHandler = handler;
    s_CModel->m_Context = private_data;

    return 1;
}
