/*START_DECLARATION*/
void checkbox_set_value ( GtkWidget * checkbox, guint * data, gboolean update );
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription );
GtkWidget *new_checkbox_with_title ( gchar * label, guint * data, GCallback hook);
GtkWidget *new_radiogroup_with_title (GtkWidget * parent,
				      gchar * title, gchar * choice1, gchar * choice2,
				      guint * data, GCallback hook);
GtkWidget * new_spin_button ( gint * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook );
void spin_button_set_value ( GtkWidget * spin, gdouble * value );
GtkWidget * new_stock_button_with_label ( gchar * stock_id, gchar * name, 
					  GCallback callback, gpointer data );
GtkWidget * new_stock_button_with_label_menu ( gchar * stock_id, gchar * name, 
					       GCallback callback, gpointer data );
void set_popup_position (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data);
/*END_DECLARATION*/

