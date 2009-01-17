#ifndef _GSB_SELECT_ICON_H
#define _GSB_SELECT_ICON_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gchar * gsb_select_icon_create_chaine_base64_from_pixbuf ( GdkPixbuf *pixbuf );
void gsb_select_icon_create_pixbuf_from_chaine_base64 ( gchar *str_base64 );
gchar * gsb_select_icon_create_window ( gchar * name_icon );
GdkPixbuf *gsb_select_icon_get_logo_pixbuf ( void );
void gsb_select_icon_set_logo_pixbuf ( GdkPixbuf *pixbuf );
/* END_DECLARATION */
#endif
