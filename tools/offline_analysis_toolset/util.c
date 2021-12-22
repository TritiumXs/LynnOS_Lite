#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

char *g_ramFile = "ram.bin"
int g_kernelBase = 0x40000000;

/* TODO: parse ini file: https://github.com/benhoyt/inih */
void parse_config()
{
/*
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("config", "r");
    if (fp == NULL) {
        exit(1);
	}

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }
*/
}

void *get_addr(off_t off)
{
	struct stat statbuf;
	int fd = open(g_ramFile, O_RDONLY);
	int ret;

	ret = stat(g_ramFile, &statbuf);
	void *addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	return addr + off - atoi(g_kernelBase);
}