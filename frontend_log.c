#include <stdio.h>
#include <stdarg.h>
#include "frontend_log.h"

void frontend_log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fputs("[frontend] ", stdout);
	vprintf(fmt, args);
	va_end(args);
}

void frontend_err(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fputs("[frontend] ", stderr);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
