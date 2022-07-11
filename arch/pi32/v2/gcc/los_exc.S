
.text
.global exception_irq_handler

exception_irq_handler:
    [--sp] = {reti, rete, retx, rets, psr, icfg, usp, ssp, sp}
    [--sp] = {r15-r0}

    r0 = sp // 把当前 sp 的值作为参数传递给解析函数
    call exception_analyze

1:
    goto 1b // 死循环