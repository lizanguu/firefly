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
#define MAXFNAMELEN 1024
#define SMAX 5

typedef double htk_time_t;  /* 以100ns单位 */

typedef enum {HTK_FALSE=0, HTK_TRUE=1} htk_bool_t; 
typedef enum {HTK_FAIL=-1, HTK_OK=0} htk_return_t; 

typedef enum {
	StrCKind,
	IntCKind,
	FltCKind,
	BoolCKind,
	AnyCKind
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

void htk_error(int errcode, char *msg, ...);
int htk_get_config(char *user, htk_bool_t inc_glob, htk_conf_param_t **list, int max);
void htk_register(char *ver, char *sccs);
htk_bool_t htk_get_conf_int(htk_conf_param_t **list, int size, char *name, int *ival);
htk_bool_t htk_get_conf_bool(htk_conf_param_t **list, int size, char *name, htk_bool_t *b);

#endif

