/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "osTest.h"
#include "it_los_hwi.h"


static VOID TaskF01(VOID)
{
}

VOID ItHwiM3B001F001()
{
    return;
}

UINT32 g_testTaskHwi = 0xFFFF;

static UINT32 Testcase(VOID)
{
    UINT32           ret;
    HwiPrio      hwiPrio = 2;
    HwiMode       mode    = 0;
    HwiIrqParam irqParam;
    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.arg = 0;
    TskInitParam task1 = {0};

    task1.pfnTaskEntry = (TskEntryFunc)TaskF01;
    task1.pcName       = "HWI_035";
    task1.taskPrio   = 4; // set new task priority is 4
    task1.stackSize  = 0x200;
    task1.resved   = LOS_TASK_STATUS_DETACHED;

    ret = LOS_TaskCreate(&g_testTaskHwi, &task1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_HwiCreate(HWI_NUM_INT0, hwiPrio, mode, (HwiProcFunc)ItHwiM3B001F001, &irqParam);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;

    LOS_TaskDelete(g_testTaskHwi);
    TestHwiDelete(HWI_NUM_TEST);

    return LOS_OK;
}
 /*
**********
Testcase brief in English
**********
*/

VOID LltLosHwi035(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("LltLosHwi035", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
