#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/syscall.h>
#include <errno.h>
#include <wait.h>
#include "err.h"
#include "test.h"

static int NUM_PAGE = 10;
static int print_maps(void);

static void pr_errno(const char *msg)
{
	printf("error: %s, %s", strerror(errno), msg);
}

static void pr_err(const char *msg)
{
	printf("error: %s", msg);
}

/* prints a region with ".........." */
static int test_1(void)
{
	char  *buf = NULL;
	int   size = NUM_PAGE * PAGE_SIZE, ret = 0;

	buf = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (buf == MAP_FAILED) {
		pr_errno(NULL);
		return -1;
	}

	print_maps();

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}

	return 0;
}

/* prints a region with "1111111111" */
static int test_2(void)
{
	char  *buf = NULL;
	int   size = NUM_PAGE * PAGE_SIZE, ret = 0, i;

	buf = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (buf == MAP_FAILED) {
		pr_errno(NULL);
		return -1;
	}

	for (i = 0; i < NUM_PAGE; i++)
		buf[i * PAGE_SIZE] = 0;

	print_maps();

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}
	return 0;
}

/* prints a region with ".1.1.1.1.1" */
static int test_3(void)
{
	char  *buf = NULL;
	int   size = NUM_PAGE * PAGE_SIZE, ret = 0, i = 0;

	buf = mmap(NULL, size, PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (buf == MAP_FAILED) {
		pr_errno(NULL);
		return -1;
	}

	for (i = 1; i < NUM_PAGE; i += 2)
		buf[i * PAGE_SIZE] = 0;

	print_maps();

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}
	return 0;
}

/* prints a region with "22222....." */
static int test_4(void)
{
	char  *buf = NULL;
	int   size = NUM_PAGE * PAGE_SIZE, ret = 0, i;
	pid_t pid;

	buf = mmap(NULL, size, PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	for (i = 0; i < 5; i++)
		buf[i * PAGE_SIZE] = 0;

	pid = fork();

	if (pid == -1) { /* process error */
		perror("error");
		return -1;
	}

	if (pid == 0) {
		print_maps();
		exit(0);
	} else if (pid > 0) {
		wait(NULL);
	}

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}
	return 0;
}

/* prints a region with "1111..2222" */
static int test_5(void)
{
	char  *buf = NULL;
	int   size = NUM_PAGE * PAGE_SIZE, ret = 0, i;
	pid_t pid;

	buf = mmap(NULL, size, PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	for (i = 6; i < NUM_PAGE; i++)
		buf[i * PAGE_SIZE] = 0;

	pid = fork();

	if (pid == -1) { /* process error */
		perror("error");
		return -1;
	}

	if (pid == 0) {
		for (i = 0; i < 4; i++)
			buf[i * PAGE_SIZE] = 0;
		print_maps();
		exit(0);
	} else if (pid > 0) {
		wait(NULL);
	}

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}
	return 0;
}

/* prints a region with exactly 2000 '.' and nothing else. */
static int test_6(void)
{
	char  *buf = NULL;
	int   size = 2000 * PAGE_SIZE, ret = 0;

	buf = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (buf == MAP_FAILED) {
		pr_errno(NULL);
		return -1;
	}

	print_maps();

	ret = munmap(buf, size);

	if (ret == -1) {
		pr_errno(NULL);
		return -1;
	}
	return 0;
}

/* triggers the Linux OOM (Out-Of-Memory) killer. */
static int test_7(void)
{
	int size = 2000 * PAGE_SIZE;

	/* Not so sure it is safe or not, should test it */
	while (1)
		mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	return 0;
}

int main(int argc, char **argv)
{
	/*
	 * Change this main function as you see fit.
	 */
	printf("test 1\n");
	test_1();
	printf("test 2\n");
	test_2();
	printf("test 3\n");
	test_3();
	printf("test 4\n");
	test_4();
	printf("test 5\n");
	test_5();
	printf("test 6\n");
	test_6();
	printf("test 7\n");
	test_7();
	return 0;
}

static int print_maps(void)
{
	/*
	 * You may not modify print_maps().
	 * Every test should call print_maps() once.
	 */
	char *path;
	char str[25000];
	int fd;
	int r, w;

	path = "/proc/self/maps";
	printf("%s:\n", path);

	fd = open(path, O_RDONLY);

	if (fd < 0)
		pr_errno(path);

	r = read(fd, str, sizeof(str));

	if (r < 0)
		pr_errno("cannot read the mapping");

	if (r == sizeof(str))
		pr_err("mapping too big");

	while (r) {
		w = write(1, str, r);
		if (w < 0)
			pr_errno("cannot write to stdout");
		r -= w;
	}

	return 0;
}
