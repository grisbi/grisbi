#ifndef _ETATS_PREFS_H
#define _ETATS_PREFS_H

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define ETATS_PREFS_TYPE              (etats_prefs_get_type())
#define ETATS_PREFS(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), ETATS_PREFS_TYPE, EtatsPrefs))
#define ETATS_IS_PREFS(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PREFS_TYPE))

/* Main object structure */
typedef struct _EtatsPrefs EtatsPrefs;
typedef struct _EtatsPrefsClass EtatsPrefsClass;

struct _EtatsPrefs
{
    GtkDialog dialog;
};

struct _EtatsPrefsClass
{
    GtkDialogClass parent_class;
};

enum EtatsPrefsPageType
{
	DATE_PAGE_TYPE,
	TRANSFER_PAGE_TYPE,
	ACCOUNT_PAGE_TYPE,
	PAYEE_PAGE_TYPE,
	CATEGORY_PAGE_TYPE,
	BUDGETARY_PAGE_TYPE
};

/* START_DECLARATION */
GType		etats_prefs_get_type										(void) G_GNUC_CONST;

GtkWidget *	etats_prefs_new 											(GtkWidget *win);
gboolean 	etats_prefs_button_toggle_get_actif 						(const gchar *button_name);
gboolean 	etats_prefs_button_toggle_set_actif 						(const gchar *button_name,
																		 gboolean actif);
gint 		etats_prefs_buttons_radio_get_active_index 					(const gchar *button_name);
void 		etats_prefs_buttons_radio_set_active_index 					(const gchar *button_name,
																		 gint index);
void 		etats_prefs_free_all_var 									(void);
gboolean 	etats_prefs_left_panel_tree_view_select_last_page			(void);
void 		etats_prefs_onglet_categ_budget_check_uncheck_all 			(GtkToggleButton *togglebutton,
																		 GtkWidget *tree_view);
GSList *	etats_prefs_onglet_mode_paiement_get_list_rows_selected 	(const gchar *treeview_name);
void 		etats_prefs_onglet_mode_paiement_select_rows_from_list 		(GSList *liste,
																		 const gchar *treeview_name);

GSList *	etats_prefs_tree_view_get_list_rows_selected 				(const gchar *treeview_name);
gint 		etats_prefs_tree_view_get_single_row_selected 				(const gchar *treeview_name);
void 		etats_prefs_tree_view_select_rows_from_list 				(GSList *liste,
																		 const gchar *treeview_name,
																		 gint column);
void 		etats_prefs_tree_view_select_single_row 					(const gchar *treeview_name,
																		 gint numero);
GtkWidget *	etats_prefs_widget_get_widget_by_name 						(const gchar *parent_name,
																		 const gchar *child_name);
gboolean 	etats_prefs_widget_set_sensitive 							(const gchar *widget_name,
																		 gboolean sensitive);
gboolean	etats_prefs_left_panel_tree_view_update_style				(GtkWidget *button,
																		  gint *page_number);

/* TRANSITOIRE */
void 		new_etats_prefs_tree_view_select_rows_from_list 			(GSList *liste,
																		 GtkWidget *tree_view,
																		 gint column);
void 		new_etats_prefs_tree_view_select_single_row 				(GtkWidget *tree_view,
												 						 gint numero);
gint 		new_etats_prefs_tree_view_get_single_row_selected			(GtkWidget *tree_view);
GtkWidget *	etats_prefs_get_page_by_number								(GtkWidget *etats_prefs,
																		 gint num_page);
GSList *	new_etats_prefs_tree_view_get_list_rows_selected			(GtkWidget *tree_view);
/* END_DECLARATION */

G_END_DECLS

#endif  /* _ETATS_PREFS_H_ */
