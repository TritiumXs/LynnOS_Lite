#include "los_interrupt.h"
#include <stdarg.h>
#include "securec.h"
#include "los_context.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"
#include "hwi.h"




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

LITE_OS_SEC_BSS UINT32  g_intCount = 0;

extern VOID ArchPendSV(VOID);



extern void exception_irq_handler(void);

LITE_OS_SEC_TEXT_INIT VOID ArchHwiInit()
{
    interrupt_init();
    int cpu = 0;
    /* task switch interrupt */
    request_irq(IRQ_SOFT3_IDX, 0, ArchPendSV, 0);
    /* initialize timer interrupt */
}



UINT32 ArchIsIntActive(VOID)
{
    UINT32 icfg = 0;
    __asm__ volatile("%0 = icfg" : "=r"(icfg));
    return (icfg & 0xff) != 0;
}

UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                     HWI_PRIOR_T hwiPrio,
                     HWI_MODE_T mode,
                     HWI_PROC_FUNC handler,
                     HwiIrqParam *irqParam)
{
    UINT32 intSave;

    (VOID) irqParam;
    if (handler == NULL) {
        return OS_ERROR;
    }
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERROR;
    }
    if ((hwiPrio < OS_HWI_PRIO_LOWEST) || (hwiPrio > OS_HWI_PRIO_HIGHEST)) {
        return OS_ERROR;
    }



    request_irq(hwiNum, hwiPrio, handler, 0);


    return LOS_OK;
}

UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum,HwiIrqParam *irqParam)
{
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERROR;
    }

    unrequest_irq(hwiNum);

    return LOS_OK;
}





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
