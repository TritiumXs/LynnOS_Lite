#ifndef _LIMITS_H
#define _LIMITS_H

#include "los_config.h"

#define PATH_MAX 256
#define NAME_MAX 255
#define MQ_PRIO_MAX 1
#define PTHREAD_STACK_MIN LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE
#define IOV_MAX 1024
#define SSIZE_MAX LONG_MAX
#define PTHREAD_KEYS_MAX 128
#include_next <limits.h>

#endif
