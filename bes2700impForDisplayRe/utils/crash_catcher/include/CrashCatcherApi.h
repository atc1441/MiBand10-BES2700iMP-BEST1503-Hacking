#ifndef __CRASH_CATCHER_API_H__
#define  __CRASH_CATCHER_API_H__

/* This structure is filled in by the Hard Fault exception handler (or unit test) and then passed in as a parameter to
   CrashCatcher_Entry(). */
typedef struct
{
    uint32_t msp;
    uint32_t psp;
    uint32_t exceptionPSR;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t exceptionLR;
} CrashCatcherExceptionRegisters;

/* This structure is filled in software assert handler and then passed in as a parameter to
    AssertCatcher_Entry */
struct TaskRegs {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

typedef struct
{
    struct TaskRegs R;
    uint32_t msp;
    uint32_t psp;
    uint32_t assertPSR;
} CrashCatcherAssertRegisters;

/* The main entry point into CrashCatcher.  Is called from the HardFault exception handler and unit tests. */
void CrashCatcher_Entry(const CrashCatcherExceptionRegisters* pExceptionRegisters);

/* The main entry point When software Assert.*/
void AssertCatcher_Entry(const CrashCatcherAssertRegisters* pAssertRegisters);

#endif /* __CRASH_CATCHER_API_H__*/
