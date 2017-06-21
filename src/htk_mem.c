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
	size_t i;
	htk_block_t *p;
	unsigned char *c;

	if (trace & T_TOP)
	  printf("HMem: htk_alloc_block of %lu bytes\n", num * size);
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
static void htk_block_reorder(htk_block_t **p, size_t n)
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
	size_t i, index;

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
		if (htk_get_conf_int(cparam, num_param, "TRACE", &i))
		  trace = i;
		if (htk_get_conf_bool(cparam, num_param, "PROTECTSTAKS", &b))
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
		printf("htk_mem: create heap %s[%c] %lu %.1f %lu %lu\n", name,c,elem_size,growf,num_elem,max_elem);
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
			  printf("htk_mem: %s[M] %zu bytes at %p allocated\n", x->name, size, q);

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
			  printf("htk_new: %s[C] %zu+%zu bytes at %p allocated\n", x->name, chdr, size, q);

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
			  printf("htk_mem: %s[S] %lu bytes at %p allocated\n", x->name, size, q);
			if (x->protect)
			{
				pp = (void **)((long)q + size - sizeof(void *));
				*pp = q;	/* record alloced mem address */
			}
			return q;
	}

	return NULL;
}

void *htk_heap_calloc(htk_heap_t *x, size_t size)
{
	void *p;

	p = htk_heap_malloc(x, size);
	if (x->type == MHEAP && size == 0)
	  size = x->elem_size;

	memset(p, 0, size);

	return p;
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

void htk_heap_free(htk_heap_t *x, void *p)
{
	size_t size, num, index, chdr, *ip;
	htk_bool_t found = HTK_FALSE;
	htk_block_t *head, *cur, *prev;
	void **pp;
	unsigned char *bp;

	switch (x->type)
	{
		case MHEAP:
			/* find block contains p */
			head = cur = x->heap;
			prev = NULL;
			size = x->elem_size;
			while (cur != NULL && !found)
			{
				num = cur->num_elem;
				if (p >= cur->data && p <= (void *)((unsigned char *)cur->data + (num+1) * size))
				  found = HTK_TRUE;
				if (!found)
				{
					prev = cur;
					cur = cur->next;
				}
			}
			if (cur == NULL)
			  htk_error(5175, "htk_heap_free: item to free in MHEAP %s not found", x->name);

			/* clear used */
			index = ((size_t)p - (size_t)cur->data) / size;
			cur->used[index/8] &= ~(1 << (index & 7));

			/* set block */
			if (index < cur->first_free)
			  cur->first_free = index;
			cur->num_free++;
			x->tot_used--;

			/* delete block if not used */
			if (cur->num_free == cur->num_elem)
			{
				if (cur != head)
				  prev->next = cur->next;
				else
				  head = cur->next;
				x->heap = head;
				x->tot_alloc -= cur->num_elem;
				free(cur->data);
				free(cur->used);
				free(cur);
			}
			if (trace & T_MHP)
			  printf("htk_mem: %s[M] %zu bytes at %p freed\n", x->name, size, p);
			return;
		case MSTACK:
			cur = x->heap;
			if (x->protect)
			{	/* check p is newst alloced mem */
				if (cur->first_free > 0) /* cur is the stack top */
				  pp = (void *)((size_t)cur->data + cur->first_free - sizeof(void *));
				else
				{	/* stack top in previous block */
				  if (cur->next == NULL)
					htk_error(5175, "htk_heap_free: empty stack");
				  pp = (void *)((size_t)cur->next->data + cur->next->first_free - sizeof(void *));
				}
				if (*pp != p)
				  htk_error(-5175, "htk_heap_free: violation of stack discipline in %s [%p != %p]", x->name, *pp, p);
			}

			while (cur != NULL && !found)
			{
				num = cur->num_elem;
				found = cur->data <= p && (((void *)((unsigned char *)cur->data + num)) > p);
				if (!found)
				{	/* not in cur block, so delete it */
					x->heap = cur->next;
					x->tot_alloc -= num;
					x->tot_used -= cur->first_free;
					free(cur->data);
					free(cur);
					cur = x->heap;
					if (trace & T_STK)
					  printf("htk_mem: deleting block in %s[S]\n", x->name);
				}
			}
			if (!found)
			  htk_error(5175, "htk_heap_free: item to free in MSTACK %s not found", x->name);

			/* finally cut back the stack in the cur block */
			size = ((unsigned char *)cur->data + cur->first_free) - (unsigned char *)p;
			if (((unsigned char *)cur->data + cur->first_free) < (unsigned char *)p)
			  htk_error(5175, "htk_heap_free: item to free in MSTACK %s is above stack top", x->name);
			cur->first_free -= size;
			cur->num_free += size;
			x->tot_used -= size;

			if (trace & T_STK)
			  printf("htk_mem: %s[S] %zu bytes at %p deallocated\n", x->name, size, p);
			return;
		case CHEAP:
			chdr = htk_mround(sizeof(size_t));
			bp = (unsigned char *)p - chdr;
			ip = (size_t *)bp;
			x->tot_alloc -= (*ip + chdr);
			x->tot_used -= *ip;
			if (trace & T_CHP)
			  printf("htk_mem: %s[C] %zu+%zu bytes at %p deallocated\n", x->name, chdr, *ip, bp);
			free(bp);
			break;
	}
}

void htk_print_heap(htk_heap_t *x)
{
	char c = 0;
	int nblocks = 0;
	htk_block_t *p;

	switch (x->type)
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
	for (p = x->heap; p != NULL; p = p->next)
	  nblocks++;

	printf("nblk=%3d, size=%6zu*%-3zu, used=%9lu, alloc=%9lu : %s[%c]\n", nblocks, x->cur_elem, x->elem_size, x->tot_used, x->tot_alloc*x->elem_size, x->name, c);
	fflush(stdout);
}

void htk_print_all_heap()
{
	htk_heap_rec_t *p;

	printf("\n---------------------- heap statistics ----------------------\n");
	for (p = heap_list; p != NULL; p = p->next)
		htk_print_heap(p->heap);
	printf("\n-------------------------------------------------------------\n");
}

/* -------------------- htk_vector/htk_matrix memory management -------------------*/

/* size of vectors for creating heaps */
size_t htk_svector_elem_size(int size)
{
	return sizeof(short) * (size + 1);
}
size_t htk_ivector_elem_size(int size)
{
	return sizeof(int) * (size + 1);
}
size_t htk_vector_elem_size(int size)
{
	return sizeof(float) * (size + 1);
}
size_t htk_dvector_elem_size(int size)
{
	return sizeof(double) * (size + 1);
}
size_t htk_sdvector_elem_size(int size)
{
	/* an extra 2 * sizeof(void *) bytes prepended to hold a usage count and a hook */
	return sizeof(float) * (size + 1) + sizeof(void *) * 2;
}

htk_svector_t htk_create_svector(htk_heap_t *x, int size)
{
	htk_svector_t v;

	v = (htk_svector_t)htk_heap_malloc(x, htk_svector_elem_size(size));
	*v = size;

	return v;
}

htk_ivector_t htk_create_ivector(htk_heap_t *x, int size)
{
	htk_ivector_t v;

	v = (htk_ivector_t)htk_heap_malloc(x, htk_ivector_elem_size(size));
	*v = size;

	return v;
}

htk_vector_t htk_create_vector(htk_heap_t *x, int size)
{
	htk_vector_t v;

	v = (htk_vector_t)htk_heap_malloc(x, htk_vector_elem_size(size));
	*v = size;

	return v;
}

htk_dvector_t htk_create_dvector(htk_heap_t *x, int size)
{
	htk_dvector_t v;

	v = (htk_dvector_t)htk_heap_malloc(x, htk_dvector_elem_size(size));
	*v = size;

	return v;
}

htk_sdvector_t htk_create_sdvector(htk_heap_t *x, int size)
{
	void **p;
	int *i;
	htk_sdvector_t v;

	p = (void **)htk_heap_malloc(x, htk_sdvector_elem_size(size));
	v = (htk_sdvector_t)(p + 2);
	i = (int *)v;
	*i = size;
	htk_set_hook(v, NULL);
	htk_set_use(v, 0);

	return v;
}

int htk_svector_size(htk_svector_t v)
{
	return (int)*v;
}

int htk_ivector_size(htk_ivector_t v)
{
	return *v;
}

int htk_vector_size(htk_vector_t v)
{
	return *((int *)v);
}

int htk_dvector_size(htk_dvector_t v)
{
	return *((int *)v);
}

void htk_free_svector(htk_heap_t *x, htk_svector_t v)
{
	htk_heap_free(x, v);
}

void htk_free_ivector(htk_heap_t *x, htk_ivector_t v)
{
	htk_heap_free(x, v);
}

void htk_free_vector(htk_heap_t *x, htk_vector_t v)
{
	htk_heap_free(x, v);
}

void htk_free_dvector(htk_heap_t *x, htk_dvector_t v)
{
	htk_heap_free(x, v);
}

void htk_free_sdvector(htk_heap_t *x, htk_sdvector_t v)
{
	htk_dec_use(v);
	if (htk_get_use(v) <= 0)
	  htk_heap_free(x, (void **)(v) - 2);
}

size_t htk_matrix_elem_size(int nrows, int ncols)
{
	return htk_vector_elem_size(ncols) * nrows + (nrows + 1) * sizeof(htk_vector_t);
}

size_t htk_dmatrix_elem_size(int nrows, int ncols)
{
	return htk_mround(htk_dvector_elem_size(ncols) * nrows + (nrows + 1) * sizeof(htk_dvector_t));
}

size_t htk_sdmatrix_elem_size(int nrows, int ncols)
{
	return htk_vector_elem_size(ncols) * nrows + (nrows+3) * sizeof(htk_vector_t);
}

/* ? */
size_t htk_tmatrix_elem_size(int size)
{
	return size * (htk_vector_elem_size(0)*2 + (size+1)*sizeof(float))/2 + (size+1)*sizeof(htk_vector_t);
}

/* ? */
size_t htk_stmatrix_elem_size(int size)
{
	return size*(htk_vector_elem_size(0)*2+(size+1)*sizeof(float))/2 + (size+1)*sizeof(htk_vector_t) + 2*sizeof(void *);
}

/* matrix结构
 * m       =>  nrows pr_1 pr_2 pr_3 ... pr_nrows
 * pr_1    =>  ncols m_11     m_12     m_13     ... m_1ncols
 * pr_2    =>  ncols m_21     m_22     m_23     ... m_2ncols
 *                      ...
 * pr_nrows => ncols m_nrows1 m_nrows2 m_nrows3 ... m_nrows_ncols
 */
htk_matrix_t htk_create_matrix(htk_heap_t *x, int nrows, int ncols)
{
	size_t vsize;
	int *i, j;
	htk_vector_t *m;
	char *p;

	p = (char *)htk_heap_malloc(x, htk_matrix_elem_size(nrows, ncols));
	i = (int *)p;
	*i = nrows;
	m = (htk_vector_t *)p;
	p += (nrows+1)*sizeof(htk_vector_t);
	vsize = htk_vector_elem_size(ncols);
	for (j = 1; j <= nrows; j++, p += vsize)
	{
		i = (int *)p;
		*i = ncols;
		m[j] = (htk_vector_t)p;
	}

	return m;
}

htk_tmatrix_t htk_create_tmatrix(htk_heap_t *x, int size)
{
	char *p;
	int *i, j;
	htk_vector_t *m;

	p = (char *)htk_heap_malloc(x, htk_tmatrix_elem_size(size));
	i = (int *)p;
	*i = size;
	m = (htk_vector_t *)p;
	p += (size + 1) * sizeof(htk_vector_t);
	for (j = 1; j <= size; j++)
	{
		i = (int *)p;
		*i = j;
		m[j] = (htk_vector_t)p;
		p += htk_vector_elem_size(j);
	}

	return m;
}

htk_dmatrix_t htk_create_dmatrix(htk_heap_t *x, int nrows, int ncols)
{
	size_t vsize;
	int *i, j;
	htk_dvector_t *m;
	char *p;

	p = (char *)htk_heap_malloc(x, htk_dmatrix_elem_size(nrows, ncols));
	i = (int *)p;
	*i = nrows;
	m = (htk_dvector_t *)p;
	p += htk_mround((nrows+1)*sizeof(htk_dvector_t));
	vsize = htk_dvector_elem_size(ncols);
	for (j = 1; j <= nrows; j++, p += vsize)
	{
		i = (int *)p;
		*i = ncols;
		m[j] = (htk_dvector_t)p;
	}

	return m;
}

htk_matrix_t htk_create_sdmatrix(htk_heap_t *x, int nrows, int ncols)
{
	size_t vsize;
	int *i, j;
	htk_vector_t *m;
	char *p;

	p = (char *)htk_heap_malloc(x, htk_sdmatrix_elem_size(nrows, ncols));
	i = (int *)p;
	*i = nrows;
	vsize = htk_vector_elem_size(ncols);
	m = (htk_vector_t *)p;
	p += (nrows + 1) * sizeof(htk_vector_t);
	for (j = 1; j <= nrows; j++, p += vsize)
	{
		i = (int *)p;
		*i = ncols;
		m[j] = (htk_vector_t)p;
	}
	htk_set_hook(m, NULL);
	htk_set_use(m, 0);

	return m;
}

htk_stmatrix_t htk_create_stmatrix(htk_heap_t *x, int size)
{
	int *i, j;
	htk_vector_t *m;
	char *p;

	p = (char *)htk_heap_malloc(x, htk_stmatrix_elem_size(size)) + 2 * sizeof(void **);
	i = (int *)p;
	m = (htk_vector_t *)p;
	p += (size+1) * sizeof(htk_vector_t);
	for (j = 1; j <= size; j++)
	{
		i = (int *)p;
		*i = j;
		m[j] = (htk_vector_t)p;
		p += htk_vector_elem_size(j);
	}
	htk_set_hook(m, NULL);
	htk_set_use(m, 0);

	return m;
}

htk_bool_t htk_is_tmatrix(htk_matrix_t m)
{
	int i, n;

	n = htk_matrix_nrows(m);
	for (i = 1; i <= n; i++)
	  if (htk_vector_size(m[i]) != i)
		return HTK_FALSE;

	return HTK_TRUE;
}

int htk_matrix_nrows(htk_matrix_t m)
{
	int *nrows;

	nrows = (int *)m;
	return *nrows;
}

int htk_matrix_ncols(htk_matrix_t m)
{
	int *ncols;

	ncols = (int *)m[1];
	return *ncols;
}

int htk_dmatrix_nrows(htk_dmatrix_t m)
{
	int *nrows;

	nrows = (int *)m;

	return *nrows;
}

int htk_dmatrix_ncols(htk_dmatrix_t m)
{
	int *ncols;

	ncols = (int *)m[1];
	return *ncols;
}

int htk_tmatrix_size(htk_tmatrix_t m)
{
	int *size;

	size = (int *)m;

	return *size;
}

void htk_free_matrix(htk_heap_t *x, htk_matrix_t m)
{
	htk_heap_free(x, m);
}

void htk_free_dmatrix(htk_heap_t *x, htk_dmatrix_t m)
{
	htk_heap_free(x, m);
}
void htk_free_sdmatrix(htk_heap_t *x, htk_sdmatrix_t m)
{
	htk_dec_use(m);
	if (htk_get_use(m) <= 0)
	  htk_heap_free(x, (void **)m - 2);
}

void htk_free_tmatrix(htk_heap_t *x, htk_tmatrix_t m)
{
	htk_heap_free(x, m);
}

void htk_free_stmatrix(htk_heap_t *x, htk_stmatrix_t m)
{
	htk_dec_use(m);
	if (htk_get_use(m) == 0)
	  htk_heap_free(x, (void **)m - 2);
}

void htk_set_use(void *m, int n)
{
	void **p;

	p = (void **)m;
	--p;
	*((int *)p) = n;
}

void htk_inc_use(void *m)
{
	void **p;

	p = (void **)m;
	--p;
	++(*((int *)p));
}

void htk_dec_use(void *m)
{
	void **p;
	
	p = (void **)m;
	--p;
	--(*((int *)p));
}

int htk_get_use(void *m)
{
	void **p;

	p = (void **)m;
	--p;
	return *((int *)p);
}

void htk_set_hook(void *m, void *p)
{
	void **pp;

	pp = (void **)m;
	pp -= 2;
	*pp = p;
}

void *htk_get_hook(void *m)
{
	void **p;

	p = (void **)m;
	p -= 2;

	return *p;
}

htk_bool_t htk_isseen_v(void *m)
{
	int i;
	void **p;

	p = (void **)m;
	--p;
	i = *((int *)p);

	return i < 0;
}

void htk_touch_v(void *m)
{
	void **p;
	int i;

	p = (void **)m;
	--p;
	i = *((int *)p);
	if (i == 0)
	  *((int *)p) = INT_MIN;
	else
	  *((int *)p) = -i;
}

void htk_untouch_v(void *m)
{
	void **p;
	int i;

	p = (void **)m;
	--p;
	i = *((int *)p);
	if (i == INT_MIN)
	  *((int *)p) = 0;
	else if (i < 0)
	  *((int *)p) = -i;
}

char *htk_new_string(htk_heap_t *x, int size)
{
	char *s;

	s = (char *)htk_heap_malloc(x, size + 1);
	*s = '\0'; /* make it empty */

	return s;
}

char *htk_copy_string(htk_heap_t *x, char *s)
{
	char *t;

	t = (char *)htk_heap_malloc(x, strlen(s) + 1);
	strcpy(t, s);

	return t;
}
