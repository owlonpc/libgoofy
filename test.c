#include <stdio.h>
#include <stdlib.h>

// FIXME: only getchar is supported
#define VERSION 1

int
main(void)
{
#if VERSION == 0
	for (int c; (c = getchar()) != EOF;)
		putchar(c);
	putchar('\n');
#elif VERSION == 1
	char buf[64];
	fgets(buf, sizeof buf, stdin);
	puts(buf);
#elif VERSION == 2
	char  *buf = malloc(64);
	size_t len;
	getline(&buf, &len, stdin);
	puts(buf);
#endif

	return 0;
}
