#ifndef _ERREUR_H
#define _ERREUR_H (1)
/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* constante de debug originale, obsolete il faudrait utiliser maintenant */
/* la variable globale debugging_grisbi, voir erreur.c */
#define DEBUG utils_str_atoi (getenv ("DEBUG_GRISBI"))

/* constantes definissant le niveau de debug */
#define	DEBUG_LEVEL_ALERT			1		/* grave probleme */
#define DEBUG_LEVEL_IMPORTANT			2		/* probleme moins grave */
#define DEBUG_LEVEL_NOTICE			3		/* probleme encore moins grave :) */
#define DEBUG_LEVEL_INFO			4		/* autre information */
#define DEBUG_LEVEL_DEBUG			5		/* information de debug */
#define MAX_DEBUG_LEVEL				5		/* ignore higher debug levels */

#define devel_debug_int(x) debug_message_int("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define devel_debug_real(x) debug_message_real("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define devel_debug(x) debug_message_string("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define notice_debug(x) debug_message_string("Info",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_INFO,FALSE)
#define warning_debug(x) debug_message_string("Warning",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_NOTICE,FALSE)
#define important_debug(x) debug_message_string("Important",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_IMPORTANT,FALSE)
#define alert_debug(x) debug_message_string("Alert",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_ALERT,FALSE)
#define DEBUG_WHERE_AM_I g_strdup_printf("%s line %05d",__FILE__,__LINE__)
#define devel_print_str(x) debug_print_log_string("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x)

/* START_DECLARATION */
extern void debug_message_int ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        gint message,
                        gint level,
                        gboolean force_debug_display );
extern void debug_message_real ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        gsb_real message,
                        gint level,
                        gboolean force_debug_display);
G_MODULE_EXPORT extern void debug_message_string ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        const gchar *message,
                        gint level,
                        gboolean force_debug_display );
extern void debug_print_log_string ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        const gchar *msg );
gboolean gsb_debug_start_log ( void );
void initialize_debugging ( void );
void traitement_sigsegv ( gint signal_nb );
/* END_DECLARATION */
#endif
