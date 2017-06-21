#include "htk_shell.h"
#include "htk_mem.h"
#include "htk_math.h"

char *hmath_version = "!HVER!htk_math: 3.4.1 [CUED 12/03/09]";
char *hmath_vc_id = "$Id: htk_math.c,v 1.1.1.1 2006/10/11 09:54:58 jal58 Exp $";

static int trace = 0;

static htk_conf_param_t *cpram[MAXGLOBS];
static int num_param = 0;

void htk_init_math();

/* ---------------- vector oriented routines --------------- */

/* zero the elements of v */
void htk_zero_svector(htk_svector_t v)
{
	int i, n;

	n = htk_svector_size(v);
	for (i = 1; i <= n; i++)
	  v[i] = 0;
}
void htk_zero_ivector(htk_ivector_t v)
{
	int i, n;

	n = htk_ivector_size(v);
	for (i = 1; i <= n; i++)
	  v[i] = 0;
}
void htk_zero_vector(htk_vector_t v)
{
	int i, n;

	n = htk_vector_size(v);
	for (i = 1; i <= n; i++)
	  v[i] = 0.0;
}
void htk_zero_dvector(htk_dvector_t v)
{
	int i, n;

	n = htk_dvector_size(v);
	for (i = 1; i <= n; i++)
	  v[i] = 0.0;
}

/* copy v1 into v2, size must be the same */
void htk_copy_svector(htk_svector_t v1, htk_svector_t v2)
{
	int i, size1, size2;

	size1 = htk_svector_size(v1);
	size2 = htk_svector_size(v2);
	if (size1 != size2)
	  htk_error(5270, "htk_copy_svector: sizes differ %d vs %d", size1, size2);
	for (i = 1; i <= size1; i++)
		v2[i] = v1[i];
}
void htk_copy_ivector(htk_ivector_t v1, htk_ivector_t v2)
{
	int i, size1, size2;

	size1 = htk_ivector_size(v1);
	size2 = htk_ivector_size(v2);
	if (size1 != size2)
	  htk_error(5270, "htk_copy_ivector: sizes differ %d vs %d", size1, size2);
	for (i = 1; i <= size1; i++)
		v2[i] = v1[i];
}
void htk_copy_vector(htk_vector_t v1, htk_vector_t v2)
{
	int i, size1, size2;

	size1 = htk_vector_size(v1);
	size2 = htk_vector_size(v2);
	if (size1 != size2)
	  htk_error(5270, "htk_copy_vector: sizes differ %d vs %d", size1, size2);
	for (i = 1; i <= size1; i++)
		v2[i] = v1[i];
}
void htk_copy_dvector(htk_dvector_t v1, htk_dvector_t v2)
{
	int i, size1, size2;

	size1 = htk_dvector_size(v1);
	size2 = htk_dvector_size(v2);
	if (size1 != size2)
	  htk_error(5270, "htk_copy_dvector: sizes differ %d vs %d", size1, size2);
	for (i = 1; i <= size1; i++)
		v2[i] = v1[i];
}

/* read vector v from source in ascii or binary */
htk_bool_t htk_read_svector(htk_source_t *src, htk_svector_t v, htk_bool_t binary)
{
	return htk_read_short(src, v+1, htk_svector_size(v), binary);
}
htk_bool_t htk_read_ivector(htk_source_t *src, htk_ivector_t v, htk_bool_t binary)
{
	return htk_read_int(src, v+1, htk_ivector_size(v), binary);
}
htk_bool_t htk_read_vector(htk_source_t *src, htk_vector_t v, htk_bool_t binary)
{
	return htk_read_float(src, v+1, htk_vector_size(v), binary);
}

/* write vector v to stream f in ascii or binary */
void htk_write_svector(FILE *f, htk_svector_t v, htk_bool_t binary)
{
	htk_write_short(f, v+1, htk_svector_size(v), binary);
	if (!binary)
	  fputc('\n', f);
}
void htk_write_ivector(FILE *f, htk_ivector_t v, htk_bool_t binary)
{
	htk_write_int(f, v+1, htk_ivector_size(v), binary);
	if (!binary)
	  fputc('\n', f);
}
void htk_write_vector(FILE *f, htk_vector_t v, htk_bool_t binary)
{
	htk_write_float(f, v+1, htk_vector_size(v), binary);
	if (!binary)
	  fputc('\n', f);
}

/* printf the title followed by upto max_terms elements of v */
void htk_show_svector(char *title, htk_svector_t v, int max_terms)
{
	int i, size, maxi;

	size = maxi = htk_svector_size(v);
	if (maxi > max_terms)
	  maxi = max_terms;
	printf("%s\n   ", title);
	for (i = 1; i <= maxi; i++)
	  printf("%3d ", v[i]);
	if (maxi < size)
	  printf("...");
	printf("\n");
}
void htk_show_ivector(char *title, htk_ivector_t v, int max_terms)
{
	int i, size, maxi;

	size = maxi = htk_ivector_size(v);
	if (maxi > max_terms)
	  maxi = max_terms;
	printf("%s\n   ", title);
	for (i = 1; i <= maxi; i++)
	  printf("%5d ", v[i]);
	if (max < size)
	  printf("...");
	printf("\n");
}
void htk_show_vector(char *title, htk_vector_t v, int max_terms)
{
	int i, size, maxi;

	size = maxi = htk_vector_size(v);
	if (maxi > max_terms)
	  maxi = max_terms;
	printf("%s\n   ", title);
	for (i = 1; i <= maxi; i++)
	  printf("%8.2f ", v[i]);
	if (maxi < size)
	  printf("...");
	printf("\n");
}
void htk_show_dvector(char *title, htk_dvector_t v, int max_terms)
{
	int i, size, maxi;

	size = maxi = htk_dvector_size(v);
	if (maxi > max_terms)
	  maxi = max_terms;
	printf("%s\n   ", title);
	for (i = 1; i <= maxi; i++)
	  printf("%10.4f ", v[i]);
	if (maxi < size)
	  printf("...");
	printf("\n");
}

/* ? */
void htk_lin_tran_qua_prod(htk_matrix_t prod, htk_matrix_t a, htk_matrix_t c)
{
}

/* ----------------- matrix oriented routines ------------------ */

void htk_zero_matrix(htk_matrix_t m);
void htk_zero_dmatrix(htk_dmatrix_t m);
void htk_zero_tmatrix(htk_tmatrix_t m);

void htk_copy_matrix(htk_matrix_t m1, htk_matrix_t m2);
void htk_copy_dmatrix(htk_dmatrix_t m1, htk_dmatrix_t m2);
void htk_copy_tmatrix(htk_tmatrix_t m1, htk_tmatrix_t m2);

/* convert matrix format from m1 to m2 which must have idential dimensions */
void htk_matrix_mat2dmat(htk_matrix_t m1, htk_dmatrix_t m2);
void htk_matrix_dmat2mat(htk_dmatrix_t m1, htk_matrix_t m2);
void htk_matrix_mat2tmat(htk_matrix_t m1, htk_tmatrix_t m2);
void htk_matrix_tmat2mat(htk_tmatrix_t m1, htk_matrix_t m2);

/* read matrix from source into m using ascii or binary.
 * htk_tmatrix_t version expects m to be in upper triangular form
 * but converts to lower triangular form internally.
 */
htk_bool_t htk_read_matrix(htk_source_t *src, htk_matrix_t m, htk_bool_t binary);
htk_bool_t htk_read_tmatrix(htk_source_t *src, htk_tmatrix_t m, htk_bool_t binary);

/* write matrix to stream in ascii or binary.
 * htk_tmatrix_t version writes m in upper triangular form even though it is stored
 * in lower triangular form 
 */
void htk_write_matrix(FILE *f, htk_matrix_t m, htk_bool_t binary);
void htk_write_tmatrix(FILE *f, htk_tmatrix_t m, htk_bool_t binary);

void htk_show_matrix(char *title, htk_matrix_t m, int max_cols, int max_rows);
void htk_show_dmatrix(char *title, htk_dmatrix_t m, int max_cols, int max_rows);
void htk_show_tmatrix(char *title, htk_tmatrix_t m, int max_cols, int max_rows);

/* --------------- linear algebra routines ------------- */

/* computes inverse of c in invc and returns the log of det(c),
 * c must be positive definite.
 */
htk_log_float_t htk_cov_invert(htk_tmatrix_t c, htk_matrix_t invc);

htk_log_float_t htk_cov_det(htk_tmatrix_t c);

float htk_matrix_det(htk_matrix_t c);
double htk_dmatrix_det(htk_dmatrix_t c);

float htk_matrix_invert(htk_matrix_t c, htk_matrix_t invc);
double htk_dmatrix_invert(htk_dmatrix_t c, htk_dmatrix_t invc);
