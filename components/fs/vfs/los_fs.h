/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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

/**
 * @defgroup los_fs fs
 * @ingroup kernel
 */

#ifndef _LOS_FS_H_
#define _LOS_FS_H_

#include "los_config.h"
#include "los_memory.h"
#include "dirent.h"
#include "sys/mount.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "sys/uio.h"
#include "unistd.h"
#include <stdarg.h>
#include "vfs_maps.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOSCFG_FS_MALLOC_HOOK
#define LOSCFG_FS_MALLOC_HOOK(size) LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, size)
#endif

#ifndef LOSCFG_FS_FREE_HOOK
#define LOSCFG_FS_FREE_HOOK(ptr) LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, ptr)
#endif

struct PartitionCfg {
    /* partition low-level read func */
    int  (*readFunc)(int partition, UINT32 *offset, void *buf, UINT32 size);
    /* partition low-level write func */
    int  (*writeFunc)(int partition, UINT32 *offset, const void *buf, UINT32 size);
    /* partition low-level erase func */
    int  (*eraseFunc)(int partition, UINT32 offset, UINT32 size);

    int readSize;       /* size of a block read */
    int writeSize;      /* size of a block write */
    int blockSize;      /* size of an erasable block */
    int blockCount;     /* number of partition blocks */
    int cacheSize;      /* size of block caches */

    int partNo;         /* partition number */
    int lookaheadSize;  /* lookahead size */
    int blockCycles;    /* block cycles */
};

/*
 * @brief Divide the device into partitions.
 *
 * @param dev Device name, which customized by caller, it is recommended to
 * name it as: "emmc0", "emmc1", "flash0", etc. The name is combined with
 * "device_type" + "device_id", and the last character is device_id.
 * device_id >= 0 && device_id <= 9.
 * @param fsType Filesystem type.
 * @param lengthArray List of partition size. For example:
 *     [0x10000000, 0x2000000], 256M and 512M partitions will be created and
 *     left all will not allocated.
 * @param addrArray List of partition start addr, partition num same as lengthArray
 * @param partNum Length of 'lengthArray'.
 *
 * @return Return LOS_NOK if error. Return LOS_OK if success.
 * Partition naming rules:
 *     In the current vfs, after successfully calling the 'fdisk' hook,
 *     "partNum" partitions will be created.
 *     The partition naming rules is:
 *         The name is a combination of: 'deviceName'+'p'+'partitionId',
 *         such as "emmc0p0", "emmc0p1", "emmc0p2"...
 */
int LOS_DiskPartition(const char *dev, const char *fsType, int *lengthArray, int *addrArray,
                      int partNum);

/*
 * @brief Format a partition.
 *
 * @param partName PartitionName, following the rule of fdisk hook.
 * @param data For FatFs, the data indicates a pointer to a byte which
 * specifies combination of FAT type flags, FM_FAT, FM_FAT32, FM_EXFAT and
 * bitwise-or of these three, FM_ANY.
 *
 * @return Return LOS_NOK if error. Return LOS_OK if success.
 */
int LOS_PartitionFormat(const char *partName, char *fsType, void *data);

/*
 * @brief new file system callbacks register.
 * These callback functions are the adaptation layer implemented by the developer,
 * used to interconnect the vfs with the new file system.
 *
 * LOS_FsRegister must be called after kernel initialization is complete.
 *
 * @param fsType file system type, don't register the same type fs more than once.
 * @param fsMops mount operation of the fs.
 * @param fsFops file operation of the fs.
 * @param fsMgt management operation of the fs.
 *
 * @return Return LOS_OK if success.
 *         Return LOS_NOK if error.
 *         errno EINVAL: input errors, such as null pointers.
 *         errno ENOMEM: memory may malloc failed.
 *
 */
int LOS_FsRegister(const char *fsType, const struct MountOps *fsMops,
                   const struct FileOps *fsFops, const struct FsManagement *fsMgt);

/*
 * @brief Lock the whole filesystem to forbid filesystem access.
 *
 * @return Return LOS_NOK if error. Return LOS_OK if seccess.
 */
int LOS_FsLock(void);

/*
 * @brief Unlock the whole filesystem to allow filesystem access.
 */
void LOS_FsUnlock(void);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_FS_H_ */
