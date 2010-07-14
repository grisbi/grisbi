#ifndef _GSB_SELECT_ICON_H
#define _GSB_SELECT_ICON_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

#define LOGO_WIDTH 100
#define LOGO_HEIGHT 100

/* START_DECLARATION */
GdkPixbuf *gsb_select_icon_change_account_pixbuf ( gint account_number,
                        gchar *filename );
gchar * gsb_select_icon_create_chaine_base64_from_pixbuf ( GdkPixbuf *pixbuf );
GdkPixbuf *gsb_select_icon_create_pixbuf_from_chaine_base64 ( gchar *str_base64 );
gchar * gsb_select_icon_create_window ( gchar *name_icon );
GdkPixbuf *gsb_select_icon_get_account_pixbuf ( gint account_number );
GdkPixbuf *gsb_select_icon_get_account_pixbuf_by_ptr ( gpointer account_icon_ptr );
GdkPixbuf *gsb_select_icon_get_default_logo_pixbuf ( void );
GdkPixbuf *gsb_select_icon_get_logo_pixbuf ( void );
gboolean gsb_select_icon_init_account_variables ( void );
gint gsb_select_icon_get_no_account_by_ptr ( gpointer account_icon_ptr );
GSList *gsb_select_icon_list_accounts_icon ( void );
gboolean gsb_select_icon_init_logo_variables ( void );
gboolean gsb_select_icon_new_account_icon ( gint account_number,
                        GdkPixbuf *pixbuf );
gboolean gsb_select_icon_new_account_icon_from_file ( gint account_number,
                        const gchar *filename );
gboolean gsb_select_icon_remove_account_pixbuf ( gint account_number );
void gsb_select_icon_set_logo_pixbuf ( GdkPixbuf *pixbuf );
/* END_DECLARATION */
#endif
