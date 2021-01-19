#ifndef _GSB_SELECT_ICON_H
#define _GSB_SELECT_ICON_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gchar * 	gsb_select_icon_create_window 						(const gchar *name_icon);
GdkPixbuf *	gsb_select_icon_get_default_logo_pixbuf 			(void);
GdkPixbuf *	gsb_select_icon_get_logo_pixbuf 					(void);
gboolean 	gsb_select_icon_init_logo_variables 				(void);
GdkPixbuf *	gsb_select_icon_new_account_pixbuf_from_file 		(const gchar *filename);
void 		gsb_select_icon_set_gtk_icon_theme_path 			(void);
gchar *		gsb_select_icon_set_icon_in_user_icons_dir			(const gchar *name_icon);
void 		gsb_select_icon_set_logo_pixbuf 					(GdkPixbuf *pixbuf);
/* END_DECLARATION */
#endif
