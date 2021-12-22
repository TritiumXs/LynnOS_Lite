/* Basetype: unsigned short */
typedef unsigned short UINT16;
/* Basetype: long long unsigned int */
typedef long long unsigned int UINT64;
/* Basetype: unsigned int */
typedef unsigned int UINT32;
/* Basetype: int */
typedef int INT32;
struct LOS_DL_LIST
{
  struct LOS_DL_LIST *pstPrev; /* +0x0 */
  struct LOS_DL_LIST *pstNext; /* +0x4 */
};
typedef struct LOS_DL_LIST LOS_DL_LIST;
typedef struct 
{
  LOS_DL_LIST sortLinkNode; /* +0x0 */
  UINT64 responseTime; /* +0x8 */
  UINT32 cpuid; /* +0x10 */
} SortLinkList;
typedef unsigned int UINTPTR;
typedef void *(*TSK_ENTRY_FUNC)(UINTPTR, UINTPTR, UINTPTR, UINTPTR);
/* Basetype: char */
typedef char CHAR;
typedef struct 
{
  UINT64 allTime; /* +0x0 */
  UINT64 startTime; /* +0x8 */
  UINT64 historyTime[12]; /* +0x10 */
} OsCpupBase;
typedef long long unsigned int sigset_t;
struct sq_entry_s
{
  struct sq_entry_s *flink; /* +0x0 */
};
typedef struct sq_entry_s sq_entry_t;
struct sq_queue_s
{
  sq_entry_t *head; /* +0x0 */
  sq_entry_t *tail; /* +0x4 */
};
typedef struct sq_queue_s sq_queue_t;
typedef int pid_t;
typedef unsigned int uid_t;
union sigval
{
  int sival_int; /* +0x0 */
  void *sival_ptr; /* +0x0 */
};
/* Basetype: long int */
typedef long int clock_t;
typedef struct 
{
  int si_signo; /* +0x0 */
  int si_errno; /* +0x4 */
  int si_code; /* +0x8 */
  union 
  {
    char __pad[116]; /* +0x0 */
    struct 
    {
      union 
      {
        struct 
        {
          pid_t si_pid; /* +0x0 */
          uid_t si_uid; /* +0x4 */
        } __piduid; /* +0x0 */
        struct 
        {
          int si_timerid; /* +0x0 */
          int si_overrun; /* +0x4 */
        } __timer; /* +0x0 */
      } __first; /* +0x0 */
      union 
      {
        union sigval si_value; /* +0x0 */
        struct 
        {
          int si_status; /* +0x0 */
          clock_t si_utime; /* +0x4 */
          clock_t si_stime; /* +0x8 */
        } __sigchld; /* +0x0 */
      } __second; /* +0x8 */
    } __si_common; /* +0x0 */
  } __si_fields; /* +0xc */
} siginfo_t;
struct SigInfoListNode
{
  struct SigInfoListNode *next; /* +0x0 */
  siginfo_t info; /* +0x4 */
};
typedef struct SigInfoListNode SigInfoListNode;
typedef struct 
{
  sigset_t sigFlag; /* +0x0 */
  sigset_t sigPendFlag; /* +0x8 */
  sigset_t sigprocmask; /* +0x10 */
  sq_queue_t sigactionq; /* +0x18 */
  LOS_DL_LIST waitList; /* +0x20 */
  sigset_t sigwaitmask; /* +0x28 */
  siginfo_t sigunbinfo; /* +0x30 */
  SigInfoListNode *tmpInfoListHead; /* +0xb0 */
  unsigned int sigIntLock; /* +0xb4 */
  void *sigContext; /* +0xb8 */
  unsigned int count; /* +0xbc */
} sig_cb;
typedef struct 
{
  void *lockPtr; /* +0x0 */
  void *lockAddr; /* +0x4 */
  UINT64 waitTime; /* +0x8 */
  UINT64 holdTime; /* +0x10 */
} HeldLocks;
typedef struct 
{
  void *waitLock; /* +0x0 */
  INT32 lockDepth; /* +0x4 */
  HeldLocks heldLocks[16]; /* +0x8 */
} LockDep;
typedef struct 
{
  UINTPTR key; /* +0x0 */
  UINT32 index; /* +0x4 */
  UINT32 pid; /* +0x8 */
  LOS_DL_LIST pendList; /* +0xc */
  LOS_DL_LIST queueList; /* +0x14 */
  LOS_DL_LIST futexList; /* +0x1c */
} FutexNode;
typedef UINT32 size_t;
typedef size_t BOOL;
typedef struct 
{
  void *stackPointer; /* +0x0 */
  UINT16 taskStatus; /* +0x4 */
  UINT16 priority; /* +0x6 */
  UINT16 policy; /* +0x8 */
  UINT64 startTime; /* +0x10 */
  UINT64 irqStartTime; /* +0x18 */
  UINT32 irqUsedTime; /* +0x20 */
  UINT32 initTimeSlice; /* +0x24 */
  INT32 timeSlice; /* +0x28 */
  UINT32 waitTimes; /* +0x2c */
  SortLinkList sortList; /* +0x30 */
  UINT32 stackSize; /* +0x48 */
  UINTPTR topOfStack; /* +0x4c */
  UINT32 taskID; /* +0x50 */
  TSK_ENTRY_FUNC taskEntry; /* +0x54 */
  void *joinRetval; /* +0x58 */
  void *taskMux; /* +0x5c */
  void *taskEvent; /* +0x60 */
  UINTPTR args[4]; /* +0x64 */
  CHAR taskName[32]; /* +0x74 */
  LOS_DL_LIST pendList; /* +0x94 */
  LOS_DL_LIST threadList; /* +0x9c */
  UINT32 eventMask; /* +0xa4 */
  UINT32 eventMode; /* +0xa8 */
  UINT32 priBitMap; /* +0xac */
  OsCpupBase taskCpup; /* +0xb0 */
  INT32 errorNo; /* +0x120 */
  UINT32 signal; /* +0x124 */
  sig_cb sig; /* +0x128 */
  UINT16 currCpu; /* +0x1e8 */
  UINT16 lastCpu; /* +0x1ea */
  UINT16 cpuAffiMask; /* +0x1ec */
  UINT32 syncSignal; /* +0x1f0 */
  LockDep lockDep; /* +0x1f8 */
  UINTPTR userArea; /* +0x380 */
  UINTPTR userMapBase; /* +0x384 */
  UINT32 userMapSize; /* +0x388 */
  UINT32 processID; /* +0x38c */
  FutexNode futex; /* +0x390 */
  LOS_DL_LIST joinList; /* +0x3b4 */
  LOS_DL_LIST lockList; /* +0x3bc */
  UINTPTR waitID; /* +0x3c4 */
  UINT16 waitFlag; /* +0x3c8 */
  UINT32 ipcStatus; /* +0x3cc */
  LOS_DL_LIST msgListHead; /* +0x3d0 */
  BOOL accessMap[128]; /* +0x3d8 */
} LosTaskCB;
