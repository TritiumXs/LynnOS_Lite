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

#if (LOS_KERNEL_MULTI_HWI_TEST == 1)
#include "it_los_hwi.h"

static VOID HwiF03(VOID)
{
    TestHwiClear(HWI_NUM_TEST1);

    g_testCount++;

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 5, g_testCount); // Compare wiht the expected value 5.
    g_testCount++;

    return;
}

static VOID HwiF02(VOID)
{
    TestHwiClear(HWI_NUM_TEST2);

    g_testCount++;

    TestHwiTrigger(HWI_NUM_TEST1);

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 3, g_testCount); // Compare wiht the expected value 3.
    g_testCount++;

    return;
}

static VOID HwiF01(VOID)
{
    TestHwiClear(HWI_NUM_TEST3);

    g_testCount++;

    TestHwiTrigger(HWI_NUM_TEST2);

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 1, g_testCount);
    g_testCount++;

    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 3;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    g_testCount = 0;

    ret = LOS_HwiCreate(HWI_NUM_TEST3, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, &irqParam);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    hwiPrio = 2; // 2, set new hwi priority
    ret = LOS_HwiCreate(HWI_NUM_TEST2, hwiPrio, mode, (HWI_PROC_FUNC)HwiF02, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT1);

    hwiPrio = 1;
    ret = LOS_HwiCreate(HWI_NUM_TEST1, hwiPrio, mode, (HWI_PROC_FUNC)HwiF03, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT2);

    TestHwiTrigger(HWI_NUM_TEST3);

    ICUNIT_GOTO_EQUAL(g_testCount, 6, g_testCount, EXIT3); // Compare wiht the expected value 6.

EXIT3:
    TestHwiDelete(HWI_NUM_TEST1);
    TestHwiDelete(HWI_NUM_TEST2);
    TestHwiDelete(HWI_NUM_TEST3);
    return LOS_OK;

EXIT2:
    TestHwiDelete(HWI_NUM_TEST2);
    TestHwiDelete(HWI_NUM_TEST3);
    return LOS_OK;

EXIT1:
    TestHwiDelete(HWI_NUM_TEST3);
    return LOS_OK;
}

VOID ItLosHwi011(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi011", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
