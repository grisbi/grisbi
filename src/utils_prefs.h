#ifndef _UTILS_PREFS_H
#define _UTILS_PREFS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *		utils_prefs_create_combo_list_indisponible 				(void);
gboolean		utils_prefs_encryption_toggled							(GtkWidget *checkbox,
																		 gpointer data);
GtkWidget *		utils_prefs_fonts_create_button 						(gchar **fontname,
																		 GCallback hook,
																		 gpointer data);
void 			utils_prefs_gsb_file_set_modified						(void);
void			utils_prefs_left_panel_add_line 						(GtkTreeStore *tree_model,
																		 GtkWidget *notebook,
																		 GtkWidget *child,
																		 const gchar *title,
																		 gint page);
gboolean 		utils_prefs_left_panel_tree_view_select_page  			(GtkWidget *tree_view,
																		 GtkWidget *notebook,
																		 gint page);
gboolean 		utils_prefs_left_panel_tree_view_selectable_func  		(GtkTreeSelection *selection,
																		 GtkTreeModel *model,
																		 GtkTreePath *path,
																		 gboolean path_currently_selected,
																		 gpointer data);
gboolean 		utils_prefs_left_panel_tree_view_selection_changed		(GtkTreeSelection *selection,
																		 GtkWidget *notebook);
GtkWidget *		utils_prefs_head_page_new_with_title_and_icon			(const gchar *title,
																		 const gchar *image_filename);
GtkWidget *		utils_prefs_paddinggrid_new_with_title                  (GtkWidget *parent,
                                                                         const gchar *title);
void 			utils_prefs_page_checkbutton_changed 					(GtkToggleButton *checkbutton,
																		 gboolean *value);
void 			utils_prefs_page_dir_chosen 							(GtkWidget *button,
																		 gchar *dirname);
void			utils_prefs_page_notebook_switch_page					(GtkNotebook *notebook,
																		 GtkWidget *page,
																		 guint page_num,
																		 guint *value);
void 			utils_prefs_spinbutton_changed 							(GtkSpinButton *spinbutton,
																		 gboolean *value);


void 			utils_prefs_close_prefs_from_theme						(void);
/* END_DECLARATION */
#endif
