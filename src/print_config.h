/* Header file for print_config.c */

gboolean print_config ( );
GtkWidget * my_file_chooser ();
void browse_file ( GtkButton *button, gpointer data );
gboolean change_print_to_file ( GtkButton *button, gpointer data );

GtkWidget * print_config_general ();
GtkWidget * print_config_paper ();
GtkWidget * print_config_appearance ();


struct paper_config 
{
  gchar * name;
  gfloat width;
  gfloat height;
};
