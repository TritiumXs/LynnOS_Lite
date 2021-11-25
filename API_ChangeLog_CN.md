### OpenHarmony-3.0-LTS版本之后的接口变更情况：

| 模块 | 接口名称 | 变更类型 | 变更描述 |变更PR |
|----|------|------|------|------|
| Kernel-Memory   | LOS_MemRegionsAdd     |Add      |支持非连续性多内存块      |[PR](https://gitee.com/openharmony/kernel_liteos_m/pulls/249)     |
|Arch    |目录调整      |Update      |将arch目录从kernel中移到根目录下      |[PR](https://gitee.com/openharmony/kernel_liteos_m/pulls/397)      |
|Components-FS    |LfsFstat      |Add      |littlefs新增FileOps->Fstat接口      |[PR](https://gitee.com/openharmony/kernel_liteos_m/pulls/409)      |