#ifndef __HTK_MEM_H__
#define __HTK_MEM_H__

typedef enum {MHEAP, MSTACK, CHEAP} htk_heap_type;

typedef struct htk_block htk_block_t;

struct htk_block {			/*	  MHEAP           MSTACK   */
	size_t num_free;		/* 空闲元素个数     剩余字节数 */
	size_t first_free;		/* 第一个空元素索引 栈顶的索引 */
	size_t num_elem;		/* 有效元素个数		使用字节数 */
	unsigned char *used;	/* 元素分布位图		未使用	   */
	void *data;				/*		     实际数据	       */
	htk_block_t *next;		/*        下一个block          */
};

typedef struct {
	char *name;				/* heap name */
	htk_heap_type type;		/* heap type */
	float growf;			
	size_t elem_size;		/* 元素大小			1 */
	size_t min_elem;		/* 初始最小元素个数    初始字节数 */
	size_t max_elem;		/* blk最大元素个数	   最大字节数 */
	size_t cur_elem;		/* 当前blk元素个数     当前字节数 */
	size_t tot_used;		/* 已使用的元素个数    已使用字节数 */
	size_t tot_alloc;		/* 已分配的元素个数    已分配字节数 */
	htk_block_t *heap;		/* block链表 */
	htk_bool_t protect;		/* MSTACK清除保护 */
} htk_heap_t;

size_t htk_mround(size_t size);

extern htk_heap_t gstack;	
extern htk_heap_t gcheap;

void htk_init_mem();

void htk_create_heap(htk_heap_t *x, char *name, htk_heap_type type, size_t elem_size,
			float growf, size_t num_elem, size_t max_elem);

void htk_reset_heap(htk_heap_t *x);
void *htk_heap_malloc(htk_heap_t *x, size_t size);
void htk_delete_heap(htk_heap_t *x);

#endif
