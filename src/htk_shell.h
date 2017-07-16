#ifndef __HTK_SHELL_H__
#define __HTK_SHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#define MAXGLOBS 256
#define MAXSTRLEN 256
#define MAXFNAMELEN 1034
#define SMAX 5

#define ALTPATHCHAR '\\'
#define PATHCHAR '/'

typedef double htk_time_t;  /* 以100ns单位 */

typedef enum {HTK_FALSE=0, HTK_TRUE=1} htk_bool_t; 
typedef enum {HTK_FAIL=-1, HTK_OK=0} htk_return_t; 

typedef enum {
    WAVE_FILTER,            /* waveforms input via htk_wave */
    PARAM_FILTER,           /* parameter files input via htk_param */
    LANG_MOD_FILTER,        /* language model files input via htk_lm */
    HMM_LIST_FILTER,        /* hmm lists input via htk_model */
    HMM_DEF_FILTER,         /* hmm definition files input via htk_model */
    LABELS_FILTER,          /* label files input via htk_label */
    NET_FILTER,             /* network file input via htk_net */
    DICT_FILTER,            /* dictionary file input via htk_dict */
    LGRAM_FILTER,          /* ngram input via htk_lg_base */
    LW_MAP_FILTER,          /* lm word map input via htk_lw_map */
    LC_MAP_FILTER,          /* lm class map input */
    LM_TEXT_FILTER,         /* lm source text input via htk_lg_prep */
    NO_FILTER,              /* direct input - no pipe */

    WAVE_OFILTER,           /* waveforms output via htk_wave */
    PARAM_OFILTER,          /* parameter files output via htk_param */
    LANG_MOD_OFILTER,       /* language mode files output via htk_lm */
    HMM_LIST_OFILTER,       /* hmm lists output via htk_model */
    HMM_DEF_OFILTER,        /* hmm definition files output via htk_model */
    LABELS_OFILTER,         /* label files output via htk_label */
    NET_OFILTER,            /* network file output via htk_net */
    DICT_OFILTER,           /* dictionary file output via htk_dict */
    LGRAM_OFILTER,          /* ngram output via htk_lg_base */
    LW_MAP_OFILTER,         /* lm word map output via htk_lw_map */
    LC_MAP_OFILTER,         /* lm class map output */
    NO_OFILTER              /* direct output - no pipe */
} htk_io_filter;

typedef struct {
    char name[256];
    FILE *f;
    htk_bool_t is_pipe;
    htk_bool_t pb_valid;
    htk_bool_t was_quoted;
    htk_bool_t was_newline;
    int putback;
    int chcount;
} htk_source_t;

typedef enum {      /* type of configuration parameter */
	STR_CKIND,
	INT_CKIND,
	FLT_CKIND,
	BOOL_CKIND,
	ANY_CKIND
} htk_conf_kind;

typedef union {
	char *s;
	int i;
	double f;
	htk_bool_t b;
} htk_conf_val_t;

typedef struct {
	char *user;
	char *name;
	htk_conf_kind kind;
	htk_conf_val_t val;
	htk_bool_t seen;
} htk_conf_param_t;

/* ------------- termination and error handling -------------- */
void htk_exit(int exitcode);
void htk_error(int errcode, char *msg, ...);
void htk_rerror(int errcode, char *msg, ...);

/* ------------- initialisation ------------- */
htk_return_t htk_set_script_file(char *fn);

extern htk_bool_t vax_order;

htk_return_t htk_init_shell(int argc, char *argv[], char *ver, char *sccs);

int htk_get_config(char *user, htk_bool_t inc_glob, htk_conf_param_t **list, int max);
void htk_register(char *ver, char *sccs);
htk_bool_t htk_get_conf_int(htk_conf_param_t **list, int size, char *name, int *ival);
htk_bool_t htk_get_conf_bool(htk_conf_param_t **list, int size, char *name, htk_bool_t *b);

typedef enum {
    HTK_SWITCH_ARG,
    HTK_STRING_ARG,
    HTK_INT_ARG,
    HTK_FLOAT_ARG,
    HTK_NO_ARG
} htk_arg_kind;

#endif

