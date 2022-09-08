/*
 *  Copyright (c) 2022 ZhuHai Jieli Technology Co.,Ltd.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

.text
.global exception_irq_handler

exception_irq_handler:
    [--sp] = {reti, rete, retx, rets, psr, icfg, usp, ssp, sp}
    [--sp] = {r15-r0}

    r0 = sp // 把当前 sp 的值作为参数传递给解析函数
    call exception_analyze

1:
    goto 1b // 死循环