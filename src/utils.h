#ifndef _UTILS_H
#define _UTILS_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean assert_account_loaded ();
gboolean desensitive_widget ( gpointer object, GtkWidget *widget );
gchar *get_gtk_run_version ( void );
void lance_mailer ( const gchar *uri );
gboolean lance_navigateur_web ( const gchar *url );
gboolean met_en_normal ( GtkWidget *event_box,
                        GdkEventMotion *event,
                        gpointer pointeur );
gboolean met_en_prelight ( GtkWidget *event_box,
                        GdkEventMotion *event,
                        gpointer pointeur );
GtkWidget *new_paddingbox_with_title (GtkWidget * parent, gboolean fill, const gchar *title);
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
                        gchar * image_filename);
gboolean radio_set_active_linked_widgets ( GtkWidget * widget );
void register_button_as_linked ( GtkWidget * widget, GtkWidget * linked );
gboolean sens_desensitive_pointeur ( GtkWidget *bouton,
                        GtkWidget *widget );
gboolean sensitive_widget ( gpointer object,
                        GtkWidget *widget );
void update_ecran ( void );
gchar *utils_get_pixmaps_dir ( void );
gchar *utils_get_plugins_dir ( void );

/* END_DECLARATION */
#endif
