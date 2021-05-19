```
.
├── components                                                  --- Optional components
│   ├── backtrace                                               --- Backtrace
│   ├── cppsupport                                              --- C++
│   ├── cpup                                                    --- CPUP
│   ├── exchook                                                 --- Exception hook
│   ├── fs                                                      --- File System
│   └── net                                                     --- Network
├── kal                                                         --- Kernel Abstraction Layer
│   ├── cmsis                                                   --- cmsis
│   └── posix                                                   --- posix
├── kernel                                                      --- Minimalistic kernel functionalities
│   ├── arch                                                    --- Support for hardware architectures
│   │   ├── arm                                                 --- arm32
│   │   │   ├── cortex-m3                                       --- cortex-m3
│   │   │   │   └── keil
│   │   │   ├── cortex-m33                                      --- cortex-m33
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   │   └── cortex-m4                                       --- cortex-m4
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   │   └── cortex-m7                                       --- cortex-m7
│   │   │   │   │── gcc
│   │   │   │   └── iar
│   │   ├── risc-v                                              --- risc-v
│   │   ├   ├── nuclei                                          --- nuclei
│   │   │   │   └── gcc
│   │   ├   └── riscv32                                         --- riscv32
│   │   │   │   └── gcc
│   │   └── include
│   │       ├── los_arch.h                                      --- Initialization
│   │       ├── los_atomic.h                                    --- Atomic operations
│   │       ├── los_context.h                                   --- Context switch
│   │       ├── los_interrupt.h                                 --- Interrupts
│   │       ├── los_mpu.h                                       --- Memory protection unit operations
│   │       └── los_timer.h                                     --- Timer operations
│   ├── include
│   │   ├── los_config.h                                        --- Configuration parameters
│   │   ├── los_event.h                                         --- Events management
│   │   ├── los_membox.h                                        --- Membox management
│   │   ├── los_memory.h                                        --- Heap memory management
│   │   ├── los_mux.h                                           --- Mutex
│   │   ├── los_queue.h                                         --- Queue
│   │   ├── los_sched.h                                         --- Scheduler
│   │   ├── los_sem.h                                           --- Semaphores
│   │   ├── los_sortlink.h                                      --- Sort link
│   │   ├── los_swtmr.h                                         --- Timer
│   │   ├── los_task.h                                          --- Tasks
│   │   └── los_tick.h                                          --- Tick
│   └── src
├── targets
│   └── targets
│       └── riscv_nuclei_demo_soc_gcc
│       ├   ├── GCC                           # compilation config
│       ├   ├── OS_CONFIG                     # board config
│       ├   ├── SoC                           # SOC codes
│       ├   └── Src                           # application codes
│       └── riscv_nuclei_gd32vf103_soc_gcc
│       └── riscv_sifive_fe310_gcc
└── utils
    ├── internal
    ├── BUILD.gn                                            --- gn build config file
    ├── los_compiler.h                                      --- Compiler configuration
    ├── los_debug.c                                         --- Debugging facilities
    ├── los_debug.h                                         ---
    ├── los_error.c                                         --- Errors codes and definitions
    ├── los_error.h                                         ---
    ├── los_hook.c                                          --- Hook function facilities
    ├── los_hook.h                                          ---
    ├── los_list.h                                          --- Doubly linked list
    └── los_reg.h                                           --- Register macros
    └── src
```
