#include "htk_shell.h"
#include "htk_mem.h"

char *hmem_version = "!HVER!HMem:   3.4.1 [CUED 12/03/09]";
char *hmem_vc_id = "$Id: HMem.c,v 1.1.1.1 2006/10/11 09:54:58 jal58 Exp $";

static int trace = 0;

#define T_TOP 0001
#define T_MHP 0002
#define T_CHP 0004
#define T_STK 0010

#define FWORD 8		/* 8字节对齐 */

static htk_conf_param_t *cparam[MAXGLOBS];
static int num_param = 0;
static htk_bool_t protect_staks = HTK_FALSE;

htk_heap_t gstack;
htk_heap_t gcheap;

typedef struct htk_heap_rec htk_heap_rec_t;

/* 内存管理链表 */
struct htk_heap_rec {
	htk_heap_t *heap;
	htk_heap_rec_t *next;
};

static htk_heap_rec_t *heap_list = NULL;

/* 将heap添加到heap链表 */
static void htk_record_heap(htk_heap_t *x)
{
	htk_heap_rec_t *p;

	if ((p = (htk_heap_rec_t *)malloc(sizeof(htk_heap_rec_t))) == NULL)
	  htk_error(5105, "htk_record_heap: can not allocate memory for htk_heap_rec");

	p->heap = x;
	p->next = heap_list;
	heap_list = p;
}

/* 从链表中删除指定heap */
static void htk_unrecord_heap(htk_heap_t *x)
{
	htk_heap_rec_t *p, *q;

	p = heap_list;
	while (p != NULL && p->heap != x)
	{
		q = p;
		p = p->next;
	}
	if (p == NULL)
	  htk_error(5171, "htk_unrecord_heap: heap %s not found", x->name);
	
	if (p == heap_list)
	  heap_list = p->next;
	else
	  q->next = p->next;

	free(p);
}

/* 返回FWORD的倍数 */
size_t htk_mround(size_t size)
{
	return ((size % FWORD) == 0) ? size : (size / FWORD + 1) * FWORD;
}

static htk_block_t *htk_alloc_block(size_t size, size_t num, htk_heap_type type)
{
	int i;
	htk_block_t *p;
	unsigned char *c;

	if (trace & T_TOP)
	  printf("HMem: htk_alloc_block of %u bytes\n", num * size);
	if ((p = (htk_block_t *)malloc(sizeof(htk_block_t))) == NULL)
	  htk_error(5105, "htk_alloc_block: can not allocate block");
	if ((p->data = (void *)malloc(num * size)) == NULL)
	  htk_error(5105, "htk_alloc_block: can not allocate block data of %u bytes", size * num);

	switch (type)
	{
		case MHEAP:
			if ((p->used = (unsigned char *)malloc(((num + 7)/8) * sizeof(unsigned char))) == NULL)
			  htk_error(5105, "htk_alloc_block: can not allocate block used array");
			for (i = 0, c = p->used; i < (num+7)/8; i++, c++)
			{
				*c = 0;
			}
			break;
		case MSTACK:
			p->used = NULL;
			break;
		default:
			htk_error(5190, "htk_alloc_block: bad type %d", type);
	}
	p->num_elem = num;
	p->num_free = num;
	p->first_free = 0;
	p->next = NULL;
	
	return p;
}

/* 将元素个数>=n的block置于首位 */
static void htk_block_reorder(htk_block_t **p, int n)
{
	htk_block_t *head, *cur, *prev;

	if (p == NULL)
	  return;

	head = cur = *p;
	prev = NULL;
	while (cur != NULL)
	{
		if (cur->num_free >= n)
		{
			if (prev != NULL)
			{
				prev->next = cur->next;
				cur->next = head;
			}
			*p = cur;
			return;
		}
		prev = cur;	
		cur = cur->next;
	}
}

static void *htk_get_elem(htk_block_t *p, size_t elem_size, htk_heap_type type)
{
	int i, index;

	switch (type)
	{
		case MHEAP:
			if (p->num_free == 0)
			  return NULL;
			index = p->first_free;
			p->used[p->first_free/8] |= 1 << (p->first_free&7);
			p->num_free--;

			if (p->num_free > 0)
			{
				for (i = p->first_free+1; i < p->num_elem; i++)
				{
					if ((p->used[i/8] & (1 << (i&7))) == 0)
					{
						p->first_free = i;
						break;
					}
				}
			}
			else
			{
				p->first_free = p->num_elem;
			}
			return (void *)((htk_block_t *)p->data + index * elem_size);
		case MSTACK:
			if (p->num_free < elem_size)
			  return NULL;
			index = p->first_free;
			p->first_free += elem_size;
			p->num_free -= elem_size;
			return (void *)((htk_block_t *)p->data + index);
		default:
			htk_error(5190, "htk_get_elem: bad type %d", type);
	}

	return NULL;
}

void htk_init_mem()
{
	int i;
	htk_bool_t b;

	htk_register(hmem_version, hmem_vc_id);
	htk_create_heap(&gstack, "global stack", MSTACK, 1, 0.0, 100000, ULONG_MAX);
	htk_create_heap(&gcheap, "global cheap", CHEAP, 1, 0.0, 100000, 0);
	num_param = htk_get_config("HMEM", HTK_TRUE, cparam, MAXGLOBS);
	if (num_param > 0) {
		if (htk_get_config(cparam, num_param, "TRACE", &i))
		  trace = i;
		if (htk_get_config(cparam, num_param, "PROTECTSTAKS", &b))
		  protect_staks = b;
	}
}

void htk_create_heap(htk_heap_t *x, char *name, htk_heap_type type, size_t elem_size,
			float growf, size_t num_elem, size_t max_elem)
{
}

void htk_reset_heap(htk_heap_t *x)
{
}
