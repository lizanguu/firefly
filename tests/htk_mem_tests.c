#include "htk_shell.h"
#include "htk_mem.h"

int main(int argc, char *argv[])
{
	int i = 71;
	printf("htk_mround(%d)=%d\n", i, htk_mround(i));

	return 0;
}
