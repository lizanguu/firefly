#ifndef __HTK_MATH_H__
#define __HTK_MATH_H__

#ifdef PI
#undef PI
#endif

#define PI 3.14159265358979
#define TPI 6.28318530717959
#define LZERO (-1.0E10)
#define LSMALL (-0.5E10)
#define MINEARG (-708.3)
#define MINLARG 2.45E-308

typedef float htk_log_float_t;
typedef double htk_log_double_t;

typedef enum {	/* 协方差矩阵类型 */
	DIAGC,		/* 对角协方差矩阵 */
	INVDIAGC,	/* 反向对角协方差矩阵 */
	FULLC,		/* inverse full rank covariance */
	XFORMC,		/* arbitrary rectangular transform */
	LLTC,		/* L' part of Choleski decomposition*/
	NULLC,		/* none - implies Eulidean in distance metrics */
	NUMCKIND	/* don't touch -- always leave as final element */
} htk_cov_kind_t;

typedef union {
	htk_svector_t var;		/* if DIAGC or INVDIAGC */
	htk_stvector_t inv;		/* if FULLC or LLTC */
	htk_smatrix_t xform;	/* if XFORMC */
} htk_covariance_t;

void htk_init_math();

/* ---------------- vector oriented routines --------------- */

/* zero the elements of v */
void htk_zero_svector(htk_svector_t v);
void htk_zero_ivector(htk_ivector_t v);
void htk_zero_vector(htk_vector_t v);
void htk_zero_dvector(htk_dvector_t v);

/* copy v1 into v2, size must be the same */
void htk_copy_svector(htk_svector_t v1, htk_svector_t v2);
void htk_copy_ivector(htk_ivector_t v1, htk_ivector_t v2);
void htk_copy_vector(htk_vector_t v1, htk_vector_t v2);
void htk_copy_dvector(htk_dvector_t v1, htk_dvector_t v2);

/* read vector v from source in ascii or binary */
htk_bool_t htk_read_svector(htk_source_t *src, htk_svector_t v, htk_bool_t binary);
htk_bool_t htk_read_ivector(htk_source_t *src, htk_ivector_t v, htk_bool_t binary);
htk_bool_t htk_read_vector(htk_source_t *src, htk_vector_t v, htk_bool_t binary);

/* write vector v to stream f in ascii or binary */
void htk_write_svector(FILE *f, htk_svector_t v, htk_bool_t binary);
void htk_write_ivector(FILE *f, htk_ivector_t v, htk_bool_t binary);
void htk_write_vector(FILE *f, htk_vector_t v, htk_bool_t binary);

/* print the title followed by upto max_terms elements of v */
void htk_show_svector(char *title, htk_svector_t v, int max_terms);
void htk_show_ivector(char *title, htk_ivector_t v, int max_terms);
void htk_show_vector(char *title, htk_vector_t v, int max_terms);
void htk_show_dvector(char *title, htk_dvector_t v, int max_terms);

/* ? */
void htk_lin_tran_qua_prod(htk_matrix_t prod, htk_matrix_t a, htk_matrix_t c);

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

#endif
