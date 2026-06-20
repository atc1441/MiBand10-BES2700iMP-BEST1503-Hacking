#ifndef _cmodel_h
#define _cmodel_h

#ifdef __cplusplus
#define externC extern "C"
#else
#define externC
#endif

typedef char            cmBool;
typedef unsigned char   cmUInt8;
typedef unsigned int    cmUInt32;
typedef size_t          cmSize;

typedef struct cmMemoryInfo
{
    cmUInt8 *   logical;
    cmUInt32    physical;
    size_t      bytes;
}
cmMemoryInfo;

typedef void (*IRQ_HANDLER)(unsigned int irq, void * context);

externC int CModelConstructor(cmSize Memory);
externC void CModelDestructor(void);
externC void CModelWriteRegister(cmUInt32 Address, cmUInt32 Data);
externC cmUInt32 CModelReadRegister(cmUInt32 Address);
externC cmMemoryInfo * CModelAllocate(cmSize Bytes, cmBool Contiguous);
externC void CModelFree(cmMemoryInfo * Memory);
externC cmUInt32 CModelPhysical(void * Logical);
externC cmBool CModelWaitInterrupt(cmUInt32 MillseSeconds);
externC cmBool CModelRegisterIrq(IRQ_HANDLER handler, void * private_data);

#endif /* _cmodel_h */
