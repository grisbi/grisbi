GtkWidget *init_menus ( GtkWidget *vbox );
void efface_derniers_fichiers_ouverts ( void );
void affiche_derniers_fichiers_ouverts ( void );
void lien_web ( GtkWidget *widget, gint origine );
void affiche_aide_locale ( gpointer null, gint origine );
gchar * menu_name ( gchar *, gchar *, gchar * );
void view_menu_cb (gpointer callback_data, guint callback_action, GtkWidget *widget);
