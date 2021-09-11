#include <sys/reboot.h>
#include "syscall.h"
#include <errno.h>
#include "los_task.h"

int reboot(int type)
{
    if (type == RB_AUTOBOOT) {
        LOS_Reboot();
        return 0;
    } else if (type == RB_POWER_OFF) {
        LOS_PowerOff();
        return 0;
    }

    errno = EINVAL;
    return -1;
}
