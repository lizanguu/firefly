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

	if (p == NULL)
	  return NULL;

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
	char c = 0;

	if (growf < 0.0)
	  htk_error(5170, "htk_create_heap: -ve grow factor in heap %s", name);
	if (num_elem > max_elem)
	  htk_error(5170, "htk_create_heap: init num elem > max elem in heap %s", name);
	if (elem_size <= 0)
	  htk_error(5170, "htk_create_heap: elem size = %u in heap %s", elem_size, name);
	if (type == MSTACK && elem_size != 1)
	  htk_error(5170, "htk_create_heap: elem size = %u in MSTACK heap %s", elem_size, name);

	x->name = (char *)malloc(strlen(name) + 1);
	strcpy(x->name, name);
	x->type = type;
	x->growf = growf;
	x->elem_size = elem_size;
	x->max_elem = max_elem;
	x->cur_elem = num_elem;
	x->min_elem = num_elem;
	x->tot_used = 0;
	x->tot_alloc = 0;
	x->heap = NULL;
	x->protect = (x == &gstack) ? HTK_FALSE : protect_staks;
	htk_record_heap(x);

	if (trace & T_TOP)
	{
		switch (type)
		{
			case MHEAP:
				c = 'M';
				break;
			case MSTACK:
				c = 'S';
				break;
			case CHEAP:
				c = 'C';
				break;
		}
		printf("htk_mem: create heap %s[%c] %u %.1f %u %u\n", name,c,elem_size,growf,num_elem,max_elem);
	}
}

void htk_delete_heap(htk_heap_t *x)
{
	if (x->type == CHEAP)
	  htk_error(5172, "htk_delete_heap: can't delete C Heap %s", x->name);

	if (trace & T_TOP)
	  printf("htk_mem: htk_delete_heap: %s\n", x->name);

	htk_reset_heap(x);
	if (x->heap != NULL)
	{
		free(x->heap->data);
		free(x->heap);
	}
	htk_unrecord_heap(x);
	free(x->name);
}

void *htk_heap_malloc(htk_heap_t *x, size_t size)
{
	void *q;
	void **pp;
	htk_block_t *newp;
	htk_bool_t nospace;
	size_t num, bytes, *ip, chdr;

	switch (x->type)
	{
		case MHEAP:
			if (size != 0 && size != x->elem_size)
			  htk_error(5173, "htk_new: MHEAP req for %u size elem from heap %s size %u", size, x->name, x->elem_size);
			nospace = x->tot_used == x->tot_alloc;
			if (nospace || (q = htk_get_elem(x->heap, x->elem_size, x->type)) == NULL)
			{
				if (!nospace)
				  htk_block_reorder(&(x->heap), 1);
				if (nospace || (q = htk_get_elem(x->heap, x->elem_size, x->type)) == NULL)
				{
					num = (size_t)((double)x->cur_elem * (x->growf + 1.0) + 0.5);
					if (num > x->max_elem)
					  num = x->max_elem; 
					newp = htk_alloc_block(x->elem_size, num, x->type);
					x->tot_alloc += num;
					x->cur_elem = num;
					newp->next = x->heap;
					x->heap = newp;
					if ((q = htk_get_elem(x->heap, x->elem_size, x->type)) == NULL)
					  htk_error(5191, "htk_new: null elem but just made block in heap %s", x->name);
				}
			}
			x->tot_used++;
			if (trace & T_MHP)
			  printf("htk_mem: %s[M] %u bytes at %p allocated\n", x->name, size, q);

			return q;

		case CHEAP:
			chdr = htk_mround(sizeof(size_t));
			q = malloc(size + chdr);
			if (q == NULL)
			  htk_error(5105, "htk_new: memory exhausted");
			x->tot_used += size;
			x->tot_alloc += size + chdr;
			ip = (size_t *)q;
			*ip = size;
			if (trace & T_CHP)
			  printf("htk_new: %s[C] %u+%u bytes at %p allocated\n", x->name, chdr, size, q);

			return (void *)((unsigned char *)q + chdr);

		case MSTACK:
			if (size > x->max_elem)
			  htk_error(5173, "htk_new: MSTACK req for %u size elem from heap %s max size %u", size, x->name, x->max_elem);
			if (x->protect)
			  size += sizeof(void *);
			size = htk_mround(size);
			if ((q = htk_get_elem(x->heap, size, x->type)) == NULL)
			{
				bytes = (size_t)((double)x->cur_elem * (x->growf + 1.0) + 0.5);
				if (bytes > x->max_elem)
				  bytes = x->max_elem;
				if (bytes < size)
				  bytes = size;
				bytes = htk_mround(bytes);
				x->cur_elem = bytes;
				newp = htk_alloc_block(1, bytes, x->type);
				x->tot_alloc += bytes;
				newp->next = x->heap;
				x->heap = newp;
				if ((q = htk_get_elem(x->heap, size, x->type)) == NULL)
				  htk_error(5191, "htk_new: null elem but just made block in heap %s", x->name);
			}
			x->tot_used += size;
			if (trace & T_STK)
			  printf("htk_mem: %s[S] %u bytes at %p allocated\n", x->name, size, q);
			if (x->protect)
			{
				pp = (void **)((long)q + size - sizeof(void *));
				*pp = q;
			}
			return q;
	}

	return NULL;
}

void htk_reset_heap(htk_heap_t *x)
{
	htk_block_t *cur, *next;

	switch (x->type)
	{
		case MHEAP:
			if (trace & T_TOP)
			  printf("htk_mem: htk_reset_heap %s[M]\n", x->name);
			cur = x->heap;
			while (cur != NULL)
			{
				next = cur->next;
				free(cur->data);
				free(cur->used);
				free(cur);
				cur = next;
			}
			x->cur_elem = x->min_elem;
			x->tot_alloc = 0;
			x->heap = NULL;
			break;
		case MSTACK:
			if (trace & T_TOP)
			  printf("htk_mem: htk_reset_heap %s[S]\n", x->name);
			cur = x->heap;
			if (cur != NULL)
			{
				while (cur->next != NULL)
				{
					next = cur->next;
					x->tot_alloc -= cur->num_elem;
					free(cur->data);
					free(cur);
					cur = next;
				}
				x->heap = cur;
			}
			x->cur_elem = x->min_elem;
			if (cur != NULL)
			{
				cur->num_free = cur->num_elem;
				cur->first_free = 0;
			}
			break;
		case CHEAP:
			htk_error(5172, "htk_reset_heap: cannot reset C Heap");
	}
	x->tot_used = 0;
}
