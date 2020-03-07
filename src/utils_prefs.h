#ifndef _UTILS_PREFS_H
#define _UTILS_PREFS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* troncature des titres de colonnes pour l'affichage de la composition */
#define	TRUNC_FORM_FIELD		20 		/* du formulaire */
#define TRUNC_LIST_COL_NAME		 8		/* de la liste des ope */

/* START_DECLARATION */
GtkWidget *		utils_prefs_create_combo_list_indisponible 				(void);
gint 			utils_prefs_dialog_msg_cancel_quit						(const gchar *msg,
																		 const gchar *hint);
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
gboolean 		utils_prefs_page_eventbox_clicked 						(GObject *eventbox,
																		 GdkEvent *event,
																		 GtkToggleButton *checkbutton);
GtkWidget *     utils_prefs_scrolled_window_new                         (GtkSizeGroup *size_group,
                                                                         GtkShadowType type,
                                                                         gint coeff_util,
                                                                         gint height);
void 			utils_prefs_spinbutton_changed 							(GtkSpinButton *spinbutton,
																		 gboolean *value);


void 			utils_prefs_close_prefs_from_theme						(void);
/* END_DECLARATION */
#endif
