#ifndef _ERREUR_H
#define _ERREUR_H (1)
/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* constantes definissant le niveau de debug 1 = alertes les plus graves */
#define DEBUG_NO_DEBUG				0		/* pas de de débug */
#define	DEBUG_LEVEL_ALERT			1		/* grave probleme */
#define DEBUG_LEVEL_IMPORTANT		2		/* probleme moins grave */
#define DEBUG_LEVEL_NOTICE			3		/* probleme encore moins grave :) */
#define DEBUG_LEVEL_INFO			4		/* autre information */
#define DEBUG_LEVEL_DEBUG			5		/* information de debug */
#define MAX_DEBUG_LEVEL				5		/* ignore higher debug levels */

#define devel_debug_int(x) debug_message_int("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define devel_debug_real(x) debug_message_real("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define devel_debug_ptr(x) debug_message_ptr("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define devel_debug(x) debug_message_string("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define notice_debug(x) debug_message_string("Info",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_INFO,FALSE)
#define warning_debug(x) debug_message_string("Warning",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_NOTICE,FALSE)
#define important_debug(x) debug_message_string("Important",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_IMPORTANT,FALSE)
#define alert_debug(x) debug_message_string("Alert",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_ALERT,FALSE)
#define DEBUG_WHERE_AM_I g_strdup_printf("%s line %05d",__FILE__,__LINE__)
#define devel_print_str(x) debug_print_log_string("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x)

/* START_DECLARATION */
gboolean	debug_get_debug_mode			(void);
void 		debug_message_int 				(const gchar *prefixe,
											 const gchar *file,
											 gint line,
											 const char *function,
											 gint message,
											 gint level,
											 gboolean force_debug_display);
void 		debug_message_real 				(const gchar *prefixe,
											 const gchar *file,
											 gint line,
											 const char *function,
											 GsbReal message,
											 gint level,
											 gboolean force_debug_display);
void 		debug_message_ptr 				(const gchar *prefixe,
											 const gchar *file,
											 gint line,
											 const char *function,
											 void * message,
											 gint level,
											 gboolean force_debug_display);
void 		debug_message_string 			(const gchar *prefixe,
											 const gchar *file,
											 gint line,
											 const char *function,
											 const gchar *message,
											 gint level,
											 gboolean force_debug_display);
void 		debug_print_log_string 			(const gchar *prefixe,
											 const gchar *file,
											 gint line,
											 const char *function,
											 const gchar *msg);
gboolean 	debug_start_log 				(void);
void 		debug_finish_log 				(void);
void 		debug_initialize_debugging 		(gint level);
void 		debug_set_cmd_line_debug_level 	(gint debug_level);
void 		debug_set_cmd_line_mode 		(gint level);
void 		debug_traitement_sigsegv 		(gint signal_nb) __attribute__ ((noreturn));
/* END_DECLARATION */
#endif
