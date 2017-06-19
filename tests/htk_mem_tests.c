#include "htk_shell.h"
#include "htk_mem.h"

int main(int argc, char *argv[])
{
	int i = 71;
	printf("htk_mround(%d)=%d\n", i, htk_mround(i));

	void *p;
	htk_heap_t heap;
	htk_create_heap(&heap, "test heap", MSTACK, 1, 0.0, 10, 10);
	p = htk_heap_malloc(&heap, 1);
	htk_delete_heap(&heap);

	return 0;
}
