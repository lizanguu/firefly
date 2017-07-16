#include "htk_shell.h"

static char *defargs[2] = {"<Uninitialised>", ""};
static char **arglist = defargs;

static htk_bool_t abort_on_error = HTK_FALSE;
static htk_bool_t show_config = HTK_FALSE;

/* ------------ version display ------------ */

typedef struct htk_version_entry htk_version_entry_t;

struct htk_version_entry
{
    char *ver;
    char *sccs;
    htk_version_entry_t *next;
};

static htk_version_entry_t *vinfo_hd = NULL;
static htk_version_entry_t *vinfo_tl = NULL;

void htk_register(char *ver, char *sccs)
{
    htk_version_entry_t *v;

    v = (htk_version_entry_t *)malloc(sizeof(htk_version_entry_t));
    v->ver = (char *)malloc(strlen(ver) + 1);
    v->sccs = (char *)malloc(strlen(sccs) + 1);
    strcpy(v->ver, ver);
    strcpy(v->sccs, sccs);
    v->next = NULL;
    if (vinfo_tl == NULL)
      vinfo_hd = v;
    else
      vinfo_tl->next = v;
    vinfo_tl = v;
}

void htk_exit(int exitcode)
{
	if (exitcode == 0 && show_config)
	  htk_print_config();
	exit(exitcode);
}

void htk_error(int errcode, char *msg, ...)
{
	va_list ap;
	FILE *f;

	fflush(stdout);
	va_start(ap, msg);
	if (errcode <= 0)
	{
		fprintf(stdout, "WARNING [%+d] ", errcode);
		f = stdout;
		vfprintf(f, msg, ap);
		va_end(ap);
		fprintf(f, " in %s\n", arglist[0]);
	}
	else
	{
		fprintf(stderr, " ERROR [%+d] ", errcode);
		f = stderr;
		vfprintf(f, msg, ap);
        va_end(ap);
		fprintf(f, "\n FATAL ERROR - Terminating program %s\n", arglist[0]);
	}
	fflush(f);
	if (errcode > 0) {
		if (abort_on_error)
		  abort();
		else
		  htk_exit(errcode);
	}
}

void htk_rerror(int errcode, char *msg, ...)
{
    fflush(stdout);
    va_start(ap, msg);
    if (errcode <= 0)
    {
        fprintf(stdout, " WARNING [%+d]  ", errcode);    
        f = stdout;
        vfprintf(f, msg, ap);
        va_end(ap);
        fprintf(f, " in %s\n", arglist[0]);
    }
    else
    {
        printf(stderr, "  ERROR [%+d]  ", errcode);
        f = stdout;
        vfprintf(f, msg, ap);
        va_end(ap);
        fprintf(f, "\n");
    }
    fflush(f);
}

/* -------------- argument processing -------------- */

static FILE *script = NULL;
static htk_bool_t was_quoted = HTK_FALSE;
static htk_bool_t script_buf_loaded = HTK_FALSE;

/* return next word from script */
static char *script_word()
{
    int i;
    int ch, qch;

    i = 0;
    ch = ' ';
    while (isspace(ch)) ch = fgetc(ch);
    if (ch == EOF)
      return NULL;
    if (ch == '\'' || c == '"')
    {
        qch = ch;
        ch = fgetc(script);
        while(ch != qch && ch != EOF)
        {
            script_buf[i++] = ch;
            ch = fgetc(script);
        }
        if (ch == EOF)
          htk_error(5051, "script_word: closing quote missing in script file");
        was_quoted = HTK_TRUE;
    }
    else
    {
        do {
            script_buf[i++] = ch;
            ch = fgetc(script);
        } while (isspace(ch) && ch != EOF);
        was_quoted = HTK_FALSE;
    }
    script_buf[i] = '\0';
    script_buf_loaded = HTK_TRUE;

    return script_buf;
}

/* check and modify fn matches machine path conventions */
static char *check_fn(char *fn)
{
    if (fn == NULL)
      return fn;
    for (s = fn; *s != 0; s++)
      if (*s == ALTPATHCHAR)
        *s = PATHCHAR;

    return fn;
}

/* open script file and count words in it */
htk_return_t htk_set_script_file(char *fn)
{
    check_fn(fn);
    if ((script = fopen(fn, "r")) == NULL)
    {
        htk_rerror(5010, "htk_set_script_file: can't open script file %s", fn);
        return HTK_FAIL;
    }
    while (script_word() != NULL)
      ++scriptcount;

    rewind(script);

    script_buf_loaded = HTK_FALSE;

    return HTK_OK;
}

/* -------------- configuration parameter file handling --------------- */

typedef struct htk_config_entry htk_config_entry_t;

struct htk_config_entry{
    htk_conf_param_t param;
    htk_config_entry_t *next;
};

static htk_config_entry_t *conf_list = NULL;
static int num_config_params = 0;
static char *cfkmap[] = {"STR_CKIND", "INT_CKIND", "FLT_CKIND", "BOOL_CKIND", "ANY_CKIND"};

static htk_bool_t read_conf_name(htk_source_t *src, char *s)
{
    int i, c;

    while (isspace(c = htk_get_ch(src)));
    if (c == EOF)
      return HTK_FALSE;
    for (i = 0; i < MAXSTRLEN; i++)
    {
        if (c == EOF || isspace(c) || !isalnum(c))
        {
            if (c == ':' || c == '=')
              htk_unget_ch(c, src);
            s[i] = '\0';
            return HTK_TRUE;
        }
        s[i] = toupper(c);
        c = htk_get_ch(src);
    }

    return HTK_FALSE;
}

static htk_config_entry_t *find_conf_entry(char *user, char *name)
{
    htk_config_entry_t *e;
    
    for (e = conf_list; e != NULL; e = e->next)
    {
        if (strcmp(e->param.name, name) == 0)
        {
            s = e->param.user;
            if (s == NULL ? (user == NULL) : (user != NULL && strcmp(s, user) == 0))
              return e;
        }
    }

    return NULL;
}

/* check s is digit */
static htk_bool_t num_head(char *s)
{
    if (*s != '\0')
      if (isdigit((int) *s))
        return HTK_TRUE;

    if (*s == '-' || *s == '+')
      if (isdigit((int)*(s+1)))
        return HTK_TRUE;

    return HTK_FALSE;
}

/* skip comments or return #include argument */
static char *parse_comment(htk_source_t *src, char *name)
{
    int c;
    const char comch = '#';
    char buf[MAXSTRLEN];

    c = htk_get_ch(src);
    while (c != EOF && (isspace(c) || c == comch))
    {
        if (c == comch)
        {
            src->was_newline = HTK_FALSE;
            htk_skip_white_space(src); 
            if (src->was_newline)
            {
                c = htk_get_ch(src);
                continue;
            }
            if (htk_read_string(src, buf) && (strcmp(buf, "include") == 0))
            {
                if (htk_read_string(src, name))
                {
                    htk_skip_line(src);
                    return name;
                }
            }
            htk_skip_line(src);
        }
        c = htk_get_ch(src);
    }
    htk_unget_ch(c, src);

    return NULL;
}

/* print the current config params */
void htk_print_config()
{
    htk_config_entry_t *e;

    printf("\n");
    if (num_config_params == 0)
      printf("no htk configuration parameters set\n");
    else
    {
        printf("htk configuration parameters[%d]\n", num_config_params);
        printf("  %-14s  %-14s  %16s\n", "module/tool", "parameter", "value");
        for (e = conf_list; e != NULL; e = e->next)
        {
            printf("%c %-14s  %-14s  ", (e->param.seen ? ' ': '#'), e->param.user==NULL?"":e->param.user, e->param.name);
            switch(e->param.kind)
            {
            case STR_CKIND:
                printf("%16s", e->param.val.s);
                break;
            case BOOL_CKIND:
                printf("%16s", e->param.val.b?"TRUE":"FALSE");
                break;
            case INT_CKIND:
                printf("%16d", e->param.val.i);
                break;
            case FLT_CKIND:
                printf("%16f", e->param.val.f);
                break;
            }
            printf("\n");
        }
    }
    printf("\n");
}

/* -------------- initialisation ------------- */

htk_return_t htk_init_shell(int argc, char *argv[], char *ver, char *sccs)
{
    char *fn;
    int i, j;
    htk_bool_t b;

    argcount = 1;
    arglist = (char **)malloc(argc * sizeof(char *));
    htk_register(ver, sccs);
    htk_register(hshell_version, hshell_vc_id);
}

int htk_get_config(char *user, htk_bool_t inc_glob, htk_conf_param_t **list, int max)
{
	return 0;
}

htk_bool_t htk_get_conf_int(htk_conf_param_t **list, int size, char *name, int *ival)
{
	return HTK_TRUE;
}

htk_bool_t htk_get_conf_bool(htk_conf_param_t **list, int size, char *name, htk_bool_t *b)
{
	return HTK_TRUE;
}

/* ------------ input files/pipes ------------ */

static char *filtermap[] = {
    "HWAVEFILTER", "HPARMFILTER", "HLANGMODFILTER", "HMMLISTFILTER",
    "HMMDEFFILTER", "HLABELFILTER", "HNETFILTER", "HDICTFILTER",
    "LGRAMFILTER", "LWMAPFILTER", "LCMAPFILTER", "LMTEXTFILTER",
    "HNOFILTER",
    "HWAVEOFILTER", "HPARMOFILTER", "HLANGMODOFILTER", "HMMLISTOFILTER",
    "HMMDEFOFILTER", "HLABELOFILTER", "HNETOFILTER", "HDICTOFILTER",
    "LGRAMOFILTER", "LWMAPOFILTER", "LCMAPOFILTER",
    "HNOOFILTER"
};

/* return true and puts filter cmd in s */
static htk_bool_t filter_set(htk_io_filter filter, char *s)
{
    char *env;

    if (env = getenv(filtermap[filter]) != NULL)
    {
        strcpy(s, env);
        return HTK_TRUE;
    }
    else if (htk_get_conf_str(cparam, nparam, filtermap[filter], s))
    {
        return HTK_TRUE;
    }
    else
    {
        return HTK_FALSE;
    }
}

/* subst '$' with fname in s */
void htk_subst_fname(char *fname, char *s)
{
    char *p;
    char buf[1028];

    while ((p = strchr(s, '$')) != NULL)
    {
        *p++ = '\0';
        strcpy(buf, s);
        strcat(buf, fname);
        strcat(buf, p);
        strcpy(s, buf);
    }
}

static int max_try = 1;

FILE *htk_fopen(char *fname, htk_io_filter filter, htk_bool_t *is_pipe)
{
    FILE *f;
    int i;
    char mode[8], cmd[1028];

    if (filter < NO_FILTER)
    {
        is_input = HTK_TRUE;
        strcpy(mode, 'r');
    }
    else
    {
        is_input = HTK_FALSE;
        strcpy(mode, 'w');
    }

    if (filter_set(filter, cmd))
    {
        htk_subst_fname(fname, cmd);
        f = (FILE *)popen(cmd, mode);
        *is_pipe = HTK_TRUE;
        if (trace & T_IOP)
          printf("htk_shell: htk_fopen - file %s %s pipe %s", fname, is_input?"<-":"->", cmd);
    }
    is_pipe = HTK_FALSE;
    strcat(mode, "b");
    for (i = 1; i <= max_try; i++)
    {
        f = fopen(fname, mode);
        if (f != NULL)
          return f;
        if (i < max_try)
          sleep(5);

        if (trace & T_IOP)
          printf("htk_shell: htk_fopen - try %d failed on %s in mode %s\n", i, fname, mode);
    }

    return NULL;
}

void htk_fclose(FILE *f, htk_bool_t is_pipe)
{
    if (is_pipe)
    {
        pclose(f);
        return;
    }
    if (fclose(f) != 0)
      htk_error(5010, "htk_fclose: closing file failed");
}

htk_return_t htk_init_source(char *fname, htk_source_t *src, htk_io_filter filter)
{
    check_fn(fname);
    strcpy(src->name, fname);
    if ((src->f = htk_fopen(fname, filter, &(src->is_pipe))) == NULL)
    {
        htk_rerror(5010, "htk_init_source: can't open source file %s", fname);
        return HTK_FAIL;
    }
    src->pb_valid = HTK_FALSE;
    src->chcount = 0;
    return HTK_OK;
}

void htk_attach_source(FILE *file, htk_source_t *src)
{
    src->f = file;
    strcpy(src->name, "attachment");
    src->is_pipe = HTK_TRUE;
    src->pb_valid = HTK_FALSE;
    src->chcount = 0;
}

void htk_close_source(htk_source_t *src)
{
    htk_fclose(src->f, src->is_pipe);
}

/* return string giving position in src */
char *htk_src_position(htk_source_t src, char *s)
{
    int i, line, col, c;
    long pos;

    if (src.is_pipe || src.chcount > 100000)
    {
        sprintf(s, "char %d in %s", src.chcount, src.name);
    }
    else
    {
        pos = ftell(src.f);
        rewind(src.f);
        for (line = 1, col = 0, i = 0; i <= pos; i++)
        {
            c = fgetc(src.f);
            if (c == '\n')
            {
                line++;
                col = 0;
            }
            else
            {
                ++col;
            }
        }
        sprintf(s, "line %d/col %d/char %ld in %s", line, col, pos, src.name);
    }

    return s;
}

int htk_get_ch(htk_source_t *src)
{
    int c;

    if (!src->pb_valid)
    {
        c = fgetc(src->f);
        ++src->chcount;
    }
    else
    {
        c = src->putback;
        src->pb_valid = HTK_FALSE;
    }

    return c;
}

/* return given character to given source */
void htk_unget_ch(int c, htk_source_t *src)
{
    if (src->pb_valid)
    {
        ungetc(src->putback, src->f);
        src->chcount--;
    }
    src->putback = c;
    src->pb_valid = HTK_TRUE;
}

htk_bool_t htk_skip_line(htk_source_t *src)
{
    int c;

    c = htk_get_ch(src);
    while (c != EOF && c != '\n')
      c = htk_get_ch(src);

    return c != EOF;
}

htk_bool_t htk_read_line(htk_source_t *src, char *s)
{
    int c;

    c = htk_get_ch(src);
    while (c != EOF && c != '\n')
    {
        *s++ = c;
        c = htk_get_ch(src);
    }
    *s = '\0':

    return c != EOF;
}

/* read to next occurrence of string */
void htk_read_until_line(htk_source_t *src, char *s)
{
    char buf[20 * MAXSTRLEN];

    do {
        if (!htk_read_line(src, buf))
          htk_error(5013, "htk_read_until_line: reached EOF while scanning for '%s'", s);
    } while (strcmp(buf, s) != 0);
}

void htk_skip_comment(htk_source_t *src)
{
    const char comch = '#';
    int c;

    c = htk_get_ch(src);
    while (c != EOF && (c == comch || isspace(c)))
    {
        if (c == comch)
        {
            while (c != EOF && c != '\n')
              c = htk_get_ch(src);
        }
        c = htk_get_ch(src);
    }
    htk_unget_ch(c, src);
}

void htk_skip_white_space(htk_source_t *src)
{
    int c;

    c = htk_get_ch(src);
    htk_unget_ch(c, src);
    if (!isspace(c))
      return;

    src->was_newline = HTK_FALSE;
    do {
        c = htk_get_ch(src);
        if (c == '\n')
          src->was_newline = HTK_TRUE;
    } while (c != EOF && isspace(c));
    if (c == EOF)
      src->was_newline = HTK_TRUE;
    htk_unget_ch(c, src);
}

/* get next string from src and store it in s */
char *htk_parse_string(char *src, char *s)
{
}
/* ------------ output routines ----------- */
