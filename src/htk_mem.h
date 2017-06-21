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
void htk_delete_heap(htk_heap_t *x);
void htk_print_heap(htk_heap_t *x);
void htk_print_all_heap();

void *htk_heap_malloc(htk_heap_t *x, size_t size);
void *htk_heap_calloc(htk_heap_t *x, size_t size);
void htk_heap_free(htk_heap_t *x, void *p);

/*---------- vector & matrix ------------*/

typedef short *htk_svector_t;
typedef int *htk_ivector_t;
typedef float *htk_vector_t;
typedef double *htk_dvector_t;
typedef htk_vector_t htk_sdvector_t;

typedef float **htk_matrix_t;
typedef htk_matrix_t htk_tmatrix_t;
typedef double **htk_dmatrix_t;
typedef htk_matrix_t htk_sdmatrix_t;
typedef htk_matrix_t htk_stmatrix_t;

size_t htk_svector_elem_size(int size);
size_t htk_ivector_elem_size(int size);
size_t htk_vector_elem_size(int size);
size_t htk_dvector_elem_size(int size);
size_t htk_sdvector_elem_size(int size);

htk_svector_t htk_create_svector(htk_heap_t *x, int size);
htk_ivector_t htk_create_ivector(htk_heap_t *x, int size);
htk_vector_t htk_create_vector(htk_heap_t *x, int size);
htk_dvector_t htk_create_dvector(htk_heap_t *x, int size);
htk_sdvector_t htk_create_sdvector(htk_heap_t *x, int size);

int htk_svector_size(htk_svector_t v);
int htk_ivector_size(htk_ivector_t v);
int htk_vector_size(htk_vector_t v);
int htk_dvector_size(htk_dvector_t v);
int htk_sdvector_size(htk_sdvector_t v);

void htk_svector_free(htk_heap_t *x, htk_svector_t v);
void htk_ivector_free(htk_heap_t *x, htk_ivector_t v);
void htk_vector_free(htk_heap_t *x, htk_vector_t v);
void htk_dvector_free(htk_heap_t *x, htk_dvector_t v);
void htk_sdvector_free(htk_heap_t *x, htk_sdvector_t v);

/* -------------- matrix --------------- */
htk_matrix_t htk_create_matrix(htk_heap_t *x, int nrows, int ncols);
htk_dmatrix_t htk_create_dmatrix(htk_heap_t *x, int nrows, int ncols);
htk_sdmatrix_t htk_create_sdmatrix(htk_heap_t *x, int nrows, int ncols);
htk_tmatrix_t htk_create_tmatrix(htk_heap_t *x, int size);
htk_stmatrix_t htk_create_sdtmatrix(htk_heap_t *x, int size);

htk_bool_t htk_is_tmatrix(htk_matrix_t m);
int htk_matrix_nrows(htk_matrix_t m);
int htk_dmatrix_nrows(htk_dmatrix_t m);
int htk_matrix_ncols(htk_matrix_t m);
int htk_dmatrix_ncols(htk_dmatrix_t m);
int htk_tmatrix_size(htk_tmatrix_t m);

void htk_free_matrix(htk_heap_t *x, htk_matrix_t m);
void htk_free_dmatrix(htk_heap_t *x, htk_dmatrix_t m);
void htk_free_sdmatrix(htk_heap_t *x, htk_sdmatrix_t m);
void htk_free_tmatrix(htk_heap_t *x, htk_tmatrix_t m);
void htk_free_sdtmatrix(htk_heap_t *x, htk_stmatrix_t m);

/* access to usage count attached to shared vector/matrix */
void htk_set_use(void *m, int n);
void htk_inc_use(void *m);
void htk_dec_use(void *m);
int htk_get_use(void *m);

/* set/clear/check nuse as "seen" flag */
htk_bool_t htk_isseen_v(void *m);
void htk_touch_v(void *m);
void htk_untouch_v(void *m);

/* access to hook attached to shared vector/matrix */
void htk_set_hook(void *m, void *p);
void *htk_get_hook(void *m);

/*------------------ string memory management --------------------*/

char *htk_new_string(htk_heap_t *x, int size);
char *htk_copy_string(htk_heap_t *x, char *s);

#endif
