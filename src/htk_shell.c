#include "htk_shell.h"

static char *defargs[2] = {"<Uninitialised>", ""};
static char **arglist = defargs;

static htk_bool_t abort_on_error = HTK_FALSE;
static htk_bool_t show_config = HTK_FALSE;

void htk_print_config()
{
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

int htk_get_config(char *user, htk_bool_t inc_glob, htk_conf_param_t **list, int max)
{
	return 0;
}

void htk_register(char *ver, char *sccs)
{
}
