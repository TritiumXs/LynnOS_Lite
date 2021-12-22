#include "LosTaskCB.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc < 5) {
		printf("Usage: xxx");
		return 1;
	}

	off_t off = atoi(argv[1]);
	
	LosTaskCB *task_addr = (LosTaskCB *)get_addr(off);
	int task_num = atoi(argv[2]);
	
	for (int i = 0; i < task_num; i++) {
		printf("taskName: %s\n", task_addr[0].taskName);
		printf("stackPointer: %x\n", task_addr[0].taskName);
		printf("taskStatus: %d\n", task_addr[0].taskStatus);
		printf("priority: %d\n", task_addr[0].priority);
		printf("policy: %d\n", task_addr[0].policy);
		printf("stackSize: %d\n", task_addr[0].stackSize);
		printf("topOfStack: %d\n", task_addr[0].topOfStack);
		printf("taskID: %d\n", task_addr[0].taskID);
		printf("taskEntry: %x(%s)\n", task_addr[0].taskEntry, get_func_by_addr(task_addr[0].taskEntry));
		printf("processID: %d\n", task_addr[0].processID);
		printf("\n");
	}

	return 0;
}
