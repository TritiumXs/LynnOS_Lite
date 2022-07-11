#include "los_context.h"
#include "los_arch_context.h"
#include "los_arch_timer.h"
#include "los_timer.h"
#include "los_task.h"
/* #include "hwi.h" */
#include "csfr.h"
/* #include <stdio.h> */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern VOID ArchHwiInit();
extern VOID OsSchedStart(VOID);
extern VOID ArchStartToRun(VOID);
extern BOOL OsSchedTaskSwitch(VOID);
extern BOOL g_taskScheduled;

LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
    ArchHwiInit();
    /* UINT32 ret = SysTickStart(OsTickHandler); */
    /* if (ret != LOS_OK) { */
        /* printf("Tick Start failed\n"); */
        /* return ; */
    /* } */


}
LITE_OS_SEC_TEXT_MINOR VOID ArchSysExit(VOID)
{
    ArchIntLock();
    while (1) {
    }
}


LITE_OS_SEC_TEXT_INIT VOID *ArchTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    TaskContext  *context = NULL;

    printf("    task stack init %x  size:%d\n", topStack,stackSize);

    /* initialize the task stack, write magic num to stack top */
    for (UINT32 index = 1; index < (stackSize / sizeof(UINT32)); index++) {
        *((UINT32 *)topStack + index) = OS_TASK_STACK_INIT;
    }
    *((UINT32 *)(topStack)) = OS_TASK_MAGIC_WORD;

    context = (TaskContext *)(((UINTPTR)topStack + stackSize) - sizeof(TaskContext));

    context->psr = 0;
    context->rets = (UINT32)ArchSysExit;
    context->reti = (UINT32)OsTaskEntry;
    context->r0 = (UINT32)taskID;

    return (VOID *)context;
}


LITE_OS_SEC_TEXT_INIT UINT32 ArchStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();

    printf("    start to run: %s, stack %x\n", g_losTask.runTask->taskName,
           g_losTask.runTask->stackPointer);
    printf("    new task %x, %s, stack %x\n",
           g_losTask.newTask, g_losTask.newTask->taskName,
           g_losTask.newTask->stackPointer);


    ArchStartToRun();
    return LOS_OK; /* never return */
}

VOID ArchTaskSchedule(VOID)
{
    // FIXME:
    // clear watch dog
    wdt_clear();
    int cpu = 0; // cpu 0
    q32DSP(cpu)->ILAT_SET |= BIT(3);
}




VOID ArchClearSchedulePending(VOID)
{
    int cpu = 0;
    q32DSP(cpu)->ILAT_CLR =  BIT(3);
}

BOOL ArchTaskSwitch(VOID)
{

    BOOL isSwitch = OsSchedTaskSwitch();
    if (isSwitch) {
        g_losTask.runTask = g_losTask.newTask;
        return TRUE;
    } else {
        return FALSE;
    }

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
