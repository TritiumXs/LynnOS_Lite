/*
* Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "los_mp.h"
#include "los_config.h"
#include "los_debug.h"

#ifdef LOSCFG_KERNEL_SMP
STATIC struct SmpOps *g_smpOps = NULL;

STATIC VOID OsSmpSecondaryInit(VOID)
{
    UINT32 cpuID = ArchCurrCpuid();

    OsIdleTaskCreate(cpuID);

    OsSchedStart();
}

VOID LOS_SmpOpsSet(struct SmpOps *ops)
{
    g_smpOps = ops;
}

LITE_OS_SEC_TEXT_INIT VOID OsSmpInit(VOID)
{
    UINT32 cpuID = 1;

    ArchIntIpiEnable();

    if (g_smpOps == NULL) {
        PRINT_ERR("Must call the interface(LOS_SmpOpsSet) to register smp operations firstly!\n");
        return;
    }

    for (; cpuID < LOSCFG_KERNEL_CORE_NUM; cpuID++) {
        ArchCpuOn(cpuID, OsSmpSecondaryInit, g_smpOps, 0);
    }
}
#endif