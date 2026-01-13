#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "capteur.h"

/* Ecrit un message dans le FILE* avec horodatage et une ligne vide pour lisibilite */
void log_append(FILE *f, const char *fmt, ...) {
    if (!f) return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] ",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);

    fprintf(f, "\n\n"); /* une ligne vide pour espacement */
    fflush(f);
}
