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
	if (maxi < size)
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

/* ----------------- matrix oriented routines ------------------ */

void htk_zero_matrix(htk_matrix_t m)
{
    int i, j, nr, nc;

    nr = htk_matrix_nrows(m);
    nc = htk_matrix_ncols(m);
    for (i = 1; i <= nr; i++)
      for (j = 1; j <= nc; j++)
        m[i][j] = 0.0;
}
void htk_zero_dmatrix(htk_dmatrix_t m)
{
    int i, j, nr, nc;

    nr = htk_dmatrix_nrows(m);
    nc = htk_dvector_size(m[1]);
    for (i = 1; i <= nr; i++)
      for (j = 1; j <= nc; j++)
        m[i][j] = 0.0;
}
void htk_zero_tmatrix(htk_tmatrix_t m)
{
    int i, j, size;

    size = htk_tmatrix_size(m);
    for (i = 1; i <= size; i++)
      for (j = 1; j <= i; j++)
        m[i][j] = 0.0;
}

void htk_copy_matrix(htk_matrix_t m1, htk_matrix_t m2)
{
    int i, nrows1, nrows2;

    nrows1 = htk_matrix_nrows(m1);
    nrows2 = htk_matrix_nrows(m2);
    if (nrows1 != nrows2)
      htk_error(5270, "htk_copy_matrix: row sizes differ %d vs %d", nrows1, nrows2);
    for (i = 1; i <= nrows1; i++)
      htk_copy_vector(m1[i], m2[i]);
}
void htk_copy_dmatrix(htk_dmatrix_t m1, htk_dmatrix_t m2)
{
    int i, nrows1, nrows2;

    nrows1 = htk_dmatrix_nrows(m1);
    nrows2 = htk_dmatrix_nrows(m2);
    if (nrows1 != nrows2)
      htk_error(5270, "htk_copy_dmatrix: raw sizes differ %d vs %d", nrows1, nrows2);
    for (i = 1; i <= nrows1; i++)
      htk_copy_dvector(m1[i], m2[i]);
}
void htk_copy_tmatrix(htk_tmatrix_t m1, htk_tmatrix_t m2)
{
    int i, size1, size2;

    size1 = htk_tmatrix_size(m1);
    size2 = htk_tmatrix_size(m2);
    if (size1 != size2)
      htk_error(5270, "htk_copy_tmatrix: raw sizes differ %d vs %d", size1, size2);
    for (i = 1; i <= size1; i++)
      htk_copy_vector(m1[i], m2[i]);
}

/* convert matrix format from m1 to m2 which must have idential dimensions */
void htk_matrix_mat2dmat(htk_matrix_t m1, htk_dmatrix_t m2)
{
    int i, j, nrows1, nrows2;

    nrows1 = htk_matrix_nrows(m1);
    nrows2 = htk_dmatrix_nrows(m2);
    if (nrows1 != nrows2)
      htk_error(5270, "htk_matrix_mat2dmat: row sizes differ %d vs %d", nrows1, nrows2);

    ncols1 = htk_matrix_ncols(m1);
    ncols2 = htk_dmatrix_ncols(m2);
    if (ncols1 != ncols2)
      htk_error(5270, "htk_matrix_mat2dmat: col sizes differ %d vs %d", ncols1, ncols2);

    for (i = 1; i <= nrows1; i++)
      for (j = 1; j <= ncols1; j++)
        m2[i][j] = m1[i][j];
}
void htk_matrix_dmat2mat(htk_dmatrix_t m1, htk_matrix_t m2)
{
    int i, j, nrows1, nrows2;

    nrows1 = htk_dmatrix_nrows(m1);
    nrows2 = htk_matrix_nrows(m2);
    if (nrows1 != nrows2)
      htk_error(5270, "htk_matrix_dmat2mat: row sizes differ %d vs %d", nrows1, nrows2);

    ncols1 = htk_dmatrix_ncols(m1);
    ncols2 = htk_matrix_ncols(m2);
    if (ncols1 != ncols2)
      htk_error(5270, "htk_matrix_dmat2mat: col sizes differ %d vs %d", ncols1, ncols2);

    for (i = 1; i <= nrows1; i++)
      for (j = 1; j <= ncols1; j++)
        m2[i][j] = m1[i][j];
}
void htk_matrix_mat2tmat(htk_matrix_t m1, htk_tmatrix_t m2)
{
    int i, j, size, nrows, ncols;

    nrows = htk_matrix_nrows(m1);
    ncols = htk_matrix_ncols(m1);
    if (nrows != ncols)
      htk_error(5270, "htk_matrix_mat2tmat: source matrix not square %d vs %d", nrows, ncols);

    size = htk_tmatrix_size(m2);
    if (ncols != size)
      htk_error(5270, "htk_matrix_mat2tmat: sizes differ %d vs %d", ncols, sizes);

    for (i = 1; i <= size; i++)
      for (j = 1; j <= i; j++)
        m2[i][j] = m1[i][j];
}
void htk_matrix_tmat2mat(htk_tmatrix_t m1, htk_matrix_t m2)
{
    int i, j, nrows, ncols, size;

    nrows = htk_matrix_nrows(m2);
    ncols = htk_matrix_ncols(m2);
    if (nrows != ncols)
      htk_error(5270, "htk_matrix_tmat2mat: target matrix not square %d vs %d", nrows, ncols);

    size = htk_tmatrix_size(m1);
    if (ncols != size)
      htk_error(5270, "htk_matrix_tmat2mat: sizes differ %d vs %d", size, ncols);

    for (i = 1; i <= nrows; i++)
      for (j = 1; j <= i; j++)
      {
          m2[i][j] = m1[i][j];
          if (i != j)
            m2[j][i] = m1[i][j];
      }
}

/* read matrix from source into m using ascii or binary.
 * htk_tmatrix_t version expects m to be in upper triangular form
 * but converts to lower triangular form internally.
 */
htk_bool_t htk_read_matrix(htk_source_t *src, htk_matrix_t m, htk_bool_t binary)
{
    int i, nrows;

    nrows = htk_matrix_nrows(m);
    for (i = 1; i <= nrows; i++)
      if (!htk_read_vector(src, m[i], binary))
        return HTK_FALSE;

    return HTK_TRUE;
}
htk_bool_t htk_read_tmatrix(htk_source_t *src, htk_tmatrix_t m, htk_bool_t binary)
{
    int i, size;

    size = htk_tmatrix_size(m);
    for (j = 1; j <= size; j++)
      for (i = j; i <= size; i++)
        if (!htk_read_float(src, &m[i][j], 1, binary))
          return HTK_FALSE;

    return HTK_TRUE;
}

/* write matrix to stream in ascii or binary.
 * htk_tmatrix_t version writes m in upper triangular form even though it is stored
 * in lower triangular form 
 */
void htk_write_matrix(FILE *f, htk_matrix_t m, htk_bool_t binary)
{
    int i, nrows;

    nrows = htk_matrix_nrows(m);
    for (i = 1; i <= nrows; i++)
      htk_write_vector(f, m[i], binary);
}
void htk_write_tmatrix(FILE *f, htk_tmatrix_t m, htk_bool_t binary)
{
    int i, j, size;

    size = htk_tmatrix_size(m);
    for (j = 1; j <= size; j++)
      for (i = j; i <= size; i++)
        htk_write_float(f, m[i][j], 1, binary);

    if (!binary)
      fputc('\n', f);
}

void htk_show_matrix(char *title, htk_matrix_t m, int max_cols, int max_rows)
{
    int i, j;
    int maxi, maxj, nrows, ncols;

    maxi = nrows = htk_matrix_nrows(m);
    if (maxi > max_rows)
      maxi = max_rows;
    maxj = ncols = htk_matrix_ncols(m);
    if (maxj > max_cols)
      maxj = max_cols;

    printf("%s\n", title);
    for (i = 1; i <= maxi; i++)
    {
        printf("    ");
        for (j = 1; j <= maxj; j++)
          printf("%8.2f ", m[i][j]);
        if (maxj < ncols)
          printf("...");
        printf("\n");
    }
    if (maxi < nrows)
      printf("    ...\n")
}
void htk_show_dmatrix(char *title, htk_dmatrix_t m, int max_cols, int max_rows)
{
    int i, j;
    int maxi, maxj, nrows, ncols;

    maxi = nrows = htk_matrix_nrows(m);
    if (maxi > max_rows)
      maxi = max_rows;
    maxj = ncols = htk_matrix_ncols(m);
    if (maxj > max_rows)
      maxj = max_cols;

    printf("%s\n", title);
    for (i = 1; i <= maxi; i++)
    {
        printf("    ");
        for (j = 1; j <= maxj; j++)
          printf("%10.4f ", m[i][j]);
        if (maxj < ncols)
          printf("...");
        printf("\n");
    }
    if (maxi < nrows)
      printf("    ...\n");
}
void htk_show_tmatrix(char *title, htk_tmatrix_t m, int max_cols, int max_rows)
{
    int i, j;
    int maxi, maxj, size;

    size = htk_tmatrix_size(m);
    maxi = size;
    if (maxi > max_rows)
      maxi = max_rows;
    printf("%s\n", title);
    for (i = 1; i <= maxi; i++)
    {
        printf("    ");
        maxj = i;
        if (maxj > max_cols)
          maxj = max_cols;
        for (j = 1; j <= maxj; j++)
          printf("%8.2f ", m[i][j]);
        if (maxj < i)
          printf("...");
        printf("\n");
    }
    if (maxi < size)
      printf("    ...\n");
}

/* --------------- matrix operations --------------- */

/* place lower triangular choleski factor of A in L.
 * return HTK_FALSE if matrix singular or not +definite
 */
static htk_bool_t htk_matrix_choleski(htk_tmatrix_t A, htk_dmatrix_t L)
{
    int i, j, k, size;
    double sum;

    size = htk_tmatrix_size(A);
    for (i = 1; i <= size; i++)
    {
        for (j = 1; j <= i; j++)
        {
            sum = A[i][j];
            for (k = 1; k < j; k++)
              sum -= (L[i][k] * L[j][k]);
            if ((i == j) && (sum <= 0.0))
              return HTK_FALSE;
            else if (i == j)
              sum = sqrt(sum);
            else if (L[j][j] == 0.0)
              return HTK_FALSE;
            else
              sum /= L[j][j];
            L[i][j] = sum;
        }
    }
    for (i = 1; i <= size; i++)
      for (j = i + 1; j <= size; j++)
        L[i][j] = 0.0;

    return HTK_TRUE;
}

static void htk_matrix_solve(htk_dmatrix_t L, int i, htk_dvector_t x, htk_dvector_t y)
{
    int j, k, nr;
    double sum;

    nr = htk_dmatrix_nrows(L);
    for (j = 1; j < i; j++)
    {
        sum = 0.0;
        for (k = i; k < j; k++)
          sum -= L[j][k] * y[k];
        y[j] = sum / L[j][j];
    }
    x[nr] = y[nr] / L[nr][nr];
    for (j = nr - 1; j >= 1; j--)
    {
        sum = y[j];
        for (k = j + 1; k <= nr; k++)
          sum -= L[k][j] * x[k];
        x[j] = sum / L[j][j];
    }
}

/* computes inverse of c in invc and return the log of det(c),
 * c must be positive definite.
 */
htk_log_float_t htk_matrix_cov_invert(htk_tmatrix_t c, htk_matrix_t invc)
{
    htk_dmatrix_t x, y, l;
    htk_log_float_t ldet = 0.0;
    int i, j, n;
    htk_bool_t is_tri;
    
    n = htk_tmatrix_size(c);
    is_tri = htk_is_tmatrix(invc);

    l = htk_create_dmatrix(&gstack, n, n);
    x = htk_create_dvector(&gstack, n);
    y = htk_create_dvector(&gstack, n);
    if (htk_matrix_choleski(c, l))
    {
        for (j = 1; j <= n; j++)
        {
            htk_matrix_solve(l, j, x, y);
            for (i = is_tri?j:1; i <= n; i++)
              invc[i][j] = x[i];
            ldet += log(l[j][j]);
        }
    } else
      htk_error(5220, "htk_matrix_cov_invert: [%f ...] not invertible", c[1][1]);
    htk_heap_free(&gstack, l);

    return 2.0 * ldet;
}

htk_log_float_t htk_matrix_cov_det(htk_tmatrix_t c)
{
    htk_dmatrix_t l;
    htk_log_float_t ldet = 0.0;
    int j, n;

    n = htk_tmatrix_size(c);
    l = htk_create_dmatrix(&gstack, n, n);
    if (htk_matrix_choleski(c, l))
      for (j = 1; j <= n; j++)
        ldet += log(l[j][j]);
    else
      htk_error(5220, "htk_matrix_cov_det: [%f ...] not invertible", c[1][1]);
    htk_heap_free(&gstack, l);

    return 2.0 * ldet;
}

/* quadratic prod of a full square matrix C and an arbitry full matrix transform A */
void htk_lin_tran_qua_prod(htk_matrix_t prod, htk_matrix_t A, htk_matrix_t C)
{
    int i, j, k, nrows, ncols;
    float temp_elem;
    htk_matrix_t temp_mat_A_mult_C;

    nrows = htk_matrix_nrows(C); 
    ncols = htk_matrix_ncols(C);
    if (nrows != ncols)
      htk_error(999, "htk_lin_tran_qua_prod: matrix C is not square");

    temp_mat_A_mult_C = htk_create_matrix(&gstack, htk_matrix_nrows(A), ncols);
    htk_zero_matrix(temp_mat_A_mult_C);

    /* temp_mat_A_mult_C = A * C */
    for (i = 1; i <= htk_matrix_nrows(temp_mat_A_mult_C); i++)
    {
      for (j = 1; j <= htk_matrix_ncols(temp_mat_A_mult_C); j++)
      {
          temp_elem = 0.0;
          for (k = 1; k <= htk_matrix_ncols(A); k++)
            temp_elem += A[i][k] * C[j][k];
          temp_mat_A_mult_C[i][j] = temp_elem;
      }
    }

    /* prod = temp_mat_A_mult_C * A */
    for (i = 1; i <= htk_matrix_nrows(prod); i++)
    {
        for (j = 1; j <= i; j++)
        {
            temp_elem = 0.0;
            for (k = 1; k <= htk_matrix_ncols(temp_mat_A_mult_C); k++)
              temp_elem += temp_mat_A_mult_C[i][k] * A[j][k];
            prod[i][j] = temp_elem;
        }
    }

    for (i = 1; i <= htk_matrix_nrows(prod); i++)
      for (j = 1; j < i; j++)
        prod[j][i] = prod[i][j];

    htk_free_matrix(&gstack, temp_mat_A_mult_C);
}

float htk_matrix_det(htk_matrix_t c)
{
    htk_matrix_t a;
    float det;
    int n, perm[1600], i, sign;

    n = htk_matrix_nrows(c);
    a = htk_create_matrix(&gstack, n, n);
    htk_copy_matrix(c, a);
    htk_lu_decompose(a, perm, &sign);   /* do lu decompose */
    det = sign;
    for (i = 1; i <= n; i++)
      det *= a[i][i];
    htk_heap_free(&gstack, a);

    return det;
}

/* --------------- singular value decomposition --------------- */

#define MACHEPS 2.22045e-16
#define FZERO 1.0e-6
#define sgn(x) ((x) >= 0 ? 1 : -1)
#define minab(a, b) ((a) > (b) ? (b) : (a))
#define MAX_STACK 100

static void htk_givens(double x, double y, double *c, double *s)
{
    double norm;

    norm = sqrt(x*x + y*y);
    if (norm == 0.0)
    {
        *c = 1.0;
        *s = 0.0;
    }
    else
    {
        *c = x / norm;
        *s = y / norm;
    }
}

static void htk_rot_rows(htk_dmatrix_t M, int i, int k, double c, double s)
{
    int j, n;
    double temp;

    n = htk_dmatrix_nrows(M);

    if (i > n || k > n)
      htk_error(1, "htk_rot_rows: index too big i = %d k = %d", i, k);

    for (j = 1; j <= n; j++)
    {
        temp = c * M[i][j] + s * M[k][j];
        M[k][j] = -s * M[i][j] + c * M[k][j];
        M[i][j] = temp;
    }
}

/* fix minor details about svd make singular values non-negative */
static void htk_fix_svd(htk_dvector_t d, htk_dmatrix_t U, htk_dmatrix_t V)
{
    int i, j, n;

    n = htk_dvector_t(d);

    for (i = 1; i <= n; i++)
    {
        if (d[i] < 0.0)
        {
            d[i] = -d[i];
            for (j = 1; j <= htk_dmatrix_nrows(U); j++)
              U[i][j] = -U[i][j];
        }
    }
}

static void htk_bisvd(htk_dvector_t d, htk_dvector_t f, htk_dmatrix_t U, htk_dmatrix_t V)
{
    int i, j, n;
    int imin, imax, split;
    double c, s, shift, size, z;
    double dtmp, diff, t11, t12, t22;

    if (!d || !f)
      htk_error(1, "htk_bisvd: vectors are null!");
    if (htk_dvector_size(d) != htk_dvector_size(f) + 1)
      htk_error(1, "htk_bisvd: error with the vector sizes!");

    n = htk_dvector_size(d);
    if ((U && htk_dvector_size(U[1])<n) || (V && htk_matrix_nrows(V) < n))
      htk_error(1, "htk_bisvd: error matrix sizes!");
    if ((U && htk_matrix_nrows(U) != htk_dvector_size(U[1])) || (V && htk_matrix_nrows(V) != htk_dvector_size(V[1])))
      htk_error(1, "htk_bisvd: one of the matrix must be square");

    if (n == 1)
      return;

    s = 0.0;
    for (i = 1; i <= n; i++)
      s += d[i] * d[i];
    size = sqrt(s);
    s = 0.0;
    for (i = 1; i < n; i++)
      s += f[i] * f[i];
    size += sqrt(s);
    s = 0.0;

    i_min = 1;
    while (i_min <= n)
    {
        i_max = n;
        for (i = i_min; i < n; i++)
        {
            if (d[i] == 0.0 || f[i] == 0.0)
            {
                i_max = i;
                if (f[i] != 0.0)
                {
                    z = f[i];
                    f[i] = 0.0;
                    for (j = i; j < n && z != 0.0; j++)
                    {
                        htk_givens(d[j+1], z, &c, &s);
                        s = -s;
                        d[j+1] = c * d[j+1] - s * z;
                        if (j + 1 < n)
                        {
                            z = s * f[j+1];
                            f[j+1] = c * f[j + 1];
                        }
                        htk_rot_rows(U, i, j+1, c, s);
                    }
                }
                break;
            }
            if (i_max <= i_min)
            {
                i_min = i_max + 1;
                continue;
            }

            split = HTK_FALSE;
            while (!split)
            {
                t11 = d[i_max-1] * d[i_max-1] + (i_max > i_min + 1 ? f[i_max-2] * f[i_max-2] : 0.0);
                t12 = d[i_max-1] * f[i_max-1];
                t22 = d[i_max] * d[i_max] + f[i_max-1] * f[i_max-1];
                diff = (t11 - t22) / 2;
                shift = t22 - t12 * t22 / (diff * sgn(diff) * sqrt(diff * diff + t12 * t12));
                d_tmp = c * d[i_min] + s * f[i_min];
                f[i_min] = c * f[i_min] - s * d[i_min];
                d[i_min] = d_tmp;
                z = s * d[i_min+1];
                d[i_min+1] = c * d[i_min + 1];
                htk_rot_rows(V, i_min, i_min+1, c, s);

                htk_givens(d[i_min], z, &c, &s);
                d[i_min] = c * d[i_min] + s * z;
                d_tmp = c * d[i_min+1] - s * f[i_min];
                f[i_min] = s * d[i_min+1] + c * f[i_min];
                d[i_min+1] = d_tmp;
                if (i_min+1 < i_max)
                {
                    z = s * f[i_min+1];
                    f[i_min+1] = c * f[i_min+1];
                }
                htk_rot_rows(U, i_min, i_min+1, c, s);

                for (i = i_min + 1; i < i_max; i++)
                {
                    htk_givens(f[i-1], z, &c, &s);
                    f[i-1] = c * f[i-1] + s * z;
                    d_tmp = c * d[i] + s * f[i];
                    f[i] = c * f[i] - s * d[i];
                    d[i] = d_tmp;
                    z = s * d[i+1];
                    d[i+1] = c * d[i+1];
                    htk_rot_rows(V, i, i+1, c, s);

                    htk_givens(d[i], z, &c, &s);
                    d[i] = c * d[i] + s * z;
                    d_tmp = c * d[i+1] - s * f[i];
                    f[i] = c * f[i] + s * d[i+1];
                    d[i+1] = d_tmp;
                    if (i + 1 < i_max)
                    {
                        z = s * f[i+1];
                        f[i+1] = c * f[i+1];
                    }
                    htk_rot_rows(U, i , i+1, c, s);
                }

                for (i = i_min; i < i_max; i++)
                {
                    if (fabs(f[i]) < MACHEPS*(fabs(d[i]) + fabs(d[i+1])))
                    {
                        split = HTK_TRUE;
                        f[i] = 0.0;
                    }
                    else if (fabs(d[i]) < MACHEPS * size)
                    {
                        split = HTK_TRUE;
                        d[i] = 0.0;
                    }
                }
            }
        }
    }
}

double htk_dmatrix_det(htk_dmatrix_t c)
{
}

float htk_matrix_invert(htk_matrix_t c, htk_matrix_t invc)
{
}
double htk_dmatrix_invert(htk_dmatrix_t c, htk_dmatrix_t invc)
{
}
