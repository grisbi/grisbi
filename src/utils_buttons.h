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
/*END_DECLARATION*/

