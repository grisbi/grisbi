#ifndef _GSB_REGEX_H
#define _GSB_REGEX_H (1)

#include <glib.h>


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void 		gsb_regex_init_variables	(void);
void 		gsb_regex_destroy			(void);
GRegex *	gsb_regex_lookup			(const gchar *);
GRegex *	gsb_regex_insert			(const gchar *,
										 const gchar *,
										 GRegexCompileFlags,
										 GRegexMatchFlags);
/* END_DECLARATION */


#endif
