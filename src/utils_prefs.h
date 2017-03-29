#ifndef _UTILS_PREFS_H
#define _UTILS_PREFS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void			utils_prefs_left_panel_add_line 						(GtkTreeStore *tree_model,
																		 GtkTreeIter *iter,
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
GtkWidget *		utils_prefs_paddinggrid_new_with_title                  (GtkWidget *parent,
                                                                         const gchar *title);
gboolean        utils_prefs_scrolled_window_allocate_size               (GtkWidget *widget,
                                                                         GtkAllocation *allocation,
                                                                         gpointer coeff_util);
GtkWidget *     utils_prefs_scrolled_window_new                         (GtkSizeGroup *size_group,
                                                                         GtkShadowType type,
                                                                         gint coeff_util,
                                                                         gint height);


/* END_DECLARATION */
#endif
