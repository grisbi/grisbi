/*START_DECLARATION*/
gdouble calcule_total_entree ( GtkWidget *entry );
void entry_set_value ( GtkWidget * entry, gchar ** value );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
GtkWidget * new_text_area ( gchar ** value, GCallback hook );
GtkWidget * new_text_entry ( gchar ** value, GCallback hook );
void text_area_set_value ( GtkWidget * text_view, gchar ** value );
/*END_DECLARATION*/

