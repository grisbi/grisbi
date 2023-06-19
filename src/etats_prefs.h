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
	BUDGET_PAGE_TYPE,
	TEXT_PAGE_TYPE,
	AMOUNT_PAGE_TYPE,
	PAYEMENT_PAGE_TYPE,
	MISC_PAGE_TYPE,
	DATA_GROUPING_TYPE,
	DATA_SEPARATION_TYPE,
	AFFICHAGE_GENERALITES_TYPE,
	AFFICHAGE_TITLES_TYPE,
	AFFICHAGE_OPERATIONS_TYPE,
	AFFICHAGE_DEVISES_TYPE
};

/* START_DECLARATION */
GType			etats_prefs_get_type										(void) G_GNUC_CONST;

EtatsPrefs *	etats_prefs_new 											(GtkWidget *win);
GtkWidget *		etats_prefs_get_page_by_number								(GtkWidget *etats_prefs,
																			 gint num_page);
GtkWidget *		etats_prefs_get_widget_by_name								(const gchar *name,
																			 GtkWidget *etats_prefs);
gboolean		etats_prefs_left_panel_tree_view_select_last_page			(GtkWidget *etats_prefs);
gboolean		etats_prefs_left_panel_tree_view_update_style				(GtkWidget *button,
																			 gint *page_number);
void			etats_prefs_onglet_categ_budget_check_uncheck_all 			(GtkToggleButton *togglebutton,
																			 GtkWidget *tree_view);
GSList *		etats_prefs_onglet_mode_paiement_get_list_rows_selected 	(GtkWidget *tree_view);
GSList *		etats_prefs_tree_view_get_list_rows_selected				(GtkWidget *tree_view);
gint 			etats_prefs_tree_view_get_single_row_selected				(GtkWidget *tree_view);
void 			etats_prefs_tree_view_select_rows_from_list 				(GSList *liste,
																			 GtkWidget *tree_view,
																			 gint column);
void 			etats_prefs_tree_view_select_single_row 					(GtkWidget *tree_view,
																			 gint numero);


/* TRANSITOIRE */
/*ONGLET_MODE_PAIEMENT*/
void			etats_prefs_initialise_onglet_mode_paiement					(GtkWidget *etats_prefs,
																			 gint report_number);
void			etats_prefs_recupere_info_onglet_mode_paiement				(GtkWidget *etats_prefs,
																			 gint report_number);
/*ONGLET_DIVERS*/
void 			etats_prefs_initialise_onglet_divers						(GtkWidget *etats_prefs,
																			 gint report_number);

void			etats_prefs_recupere_info_onglet_divers						(GtkWidget *etats_prefs,
																			 gint report_number);
/*ONGLET_DATA_GROUPING*/
void			etats_prefs_initialise_onglet_data_grouping					(GtkWidget *etats_prefs,
																			 gint report_number);
void			etats_prefs_recupere_info_onglet_data_grouping				(GtkWidget *etats_prefs,
																			 gint report_number);
/*ONGLET_DATA_SEPARATION*/
void			etats_prefs_initialise_onglet_data_separation				(GtkWidget *etats_prefs,
						 													 gint report_number);
void			etats_prefs_recupere_info_onglet_data_separation			(GtkWidget *etats_prefs,
																			 gint report_number);
/*ONGLET_AFFICHAGE_GENERALITES*/
/* END_DECLARATION */

G_END_DECLS

#endif  /* _ETATS_PREFS_H_ */
