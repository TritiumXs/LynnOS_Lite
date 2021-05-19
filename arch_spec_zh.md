```
.
├── components                                                  --- 可选组件，可裁剪，依赖kernel
│   ├── backtrace                                               --- 回溯栈支持
│   ├── cppsupport                                              --- C++支持
│   ├── cpup                                                    --- CPUP功能
│   ├── exchook                                                 --- 异常钩子
│   ├── fs                                                      --- 文件系统
│   └── net                                                     --- 网络功能
├── kal                                                         --- 内核抽象层
│   ├── cmsis                                                   --- cmsis标准支持
│   └── posix                                                   --- posix标准支持
├── kernel                                                      --- 内核最小功能集支持
│   ├── arch                                                    --- 硬件架构相关
│   │   ├── arm                                                 --- arm32架构
│   │   │   ├── cortex-m3                                       --- cortex-m3架构
│   │   │   │   └── keil
│   │   │   ├── cortex-m33                                      --- cortex-m33架构
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   │   └── cortex-m4                                       --- cortex-m4架构
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   │   └── cortex-m7                                       --- cortex-m7架构
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   ├── risc-v                                              --- risc-v架构
│   │   ├   ├── nuclei                                          --- nuclei
│   │   │   │   └── gcc
│   │   ├   └── riscv32                                         --- riscv32
│   │   │   │   └── gcc
│   │   └── include
│   │       ├── los_arch.h                                     --- 定义arch初始化
│   │       ├── los_atomic.h                                   --- 定义通用arch原子操作
│   │       ├── los_context.h                                  --- 定义通用arch上下文切换
│   │       ├── los_interrupt.h                                --- 定义通用arch中断
│   │       ├── los_mpu.h                                      --- 定义通用arch内存保护
│   │       └── los_timer.h                                    --- 定义通用arch定时器
│   ├── include
│   │   ├── los_config.h                                        --- 功能开关和配置参数
│   │   ├── los_event.h                                         --- 事件
│   │   ├── los_membox.h                                        --- 静态内存管理
│   │   ├── los_memory.h                                        --- 动态内存管理
│   │   ├── los_mux.h                                           --- 互斥锁
│   │   ├── los_queue.h                                         --- 队列
│   │   ├── los_sched.h                                         --- 调度算法
│   │   ├── los_sem.h                                           --- 信号量
│   │   ├── los_sortlink.h                                      --- 排序链表
│   │   ├── los_swtmr.h                                         --- 定时器
│   │   ├── los_task.h                                          --- 任务
│   │   └── los_tick.h                                          --- Tick时钟
   └── src
├── targets
│   └── targets
│       └── riscv_nuclei_demo_soc_gcc
│       ├   ├── GCC                           # 编译相关
│       ├   ├── OS_CONFIG                     # 开发板配置功能开关和配置参数
│       ├   ├── SoC                           # SOC相关代码
│       ├   └── Src                           # application相关代码
│       └── riscv_nuclei_gd32vf103_soc_gcc
│       └── riscv_sifive_fe310_gcc
└── utils
    ├── internal
    ├── BUILD.gn                                            --- gn构建文件
    ├── los_compiler.h                                      --- 编译工具配置，类型定义
    ├── los_debug.c                                         --- debug，printf相关
    ├── los_debug.h                                         ---
    ├── los_error.c                                         --- 错误处理
    ├── los_error.h                                         ---
    ├── los_hook.c                                          --- 钩子函数注册和调用
    ├── los_hook.h                                          ---
    ├── los_list.h                                          --- 双向链表
    └── los_reg.h                                           --- 寄存器读写宏定义
    └── src
```
