#ifndef _ETATS_PREFS_H
#define _ETATS_PREFS_H

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define ETATS_TYPE_PREFS              (etats_prefs_get_type())
#define ETATS_PREFS(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), ETATS_TYPE_PREFS, EtatsPrefs))
#define ETATS_PREFS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), ETATS_TYPE_PREFS, EtatsPrefsClass))
#define ETATS_IS_PREFS(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_TYPE_PREFS))
#define ETATS_IS_PREFS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), ETATS_TYPE_PREFS))
#define ETATS_PREFS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), ETATS_TYPE_PREFS, EtatsPrefsClass))

/* Private structure type */
typedef struct _EtatsPrefsPrivate EtatsPrefsPrivate;


/* Main object structure */
typedef struct _EtatsPrefs EtatsPrefs;

struct _EtatsPrefs
{
    GtkDialog dialog;

    /*< private > */
    EtatsPrefsPrivate *priv;
};


/* Class definition */
typedef struct _EtatsPrefsClass EtatsPrefsClass;

struct _EtatsPrefsClass
{
    GtkDialogClass parent_class;
};


/* construction */
GType etats_prefs_get_type (void) G_GNUC_CONST;
GtkWidget *etats_prefs_new (GtkWidget *parent);

gboolean etats_prefs_button_toggle_get_actif (const gchar *button_name);
gboolean etats_prefs_button_toggle_set_actif (const gchar *button_name,
                        gboolean actif);

gint etats_prefs_buttons_radio_get_active_index (const gchar *button_name);
void etats_prefs_buttons_radio_set_active_index (const gchar *button_name,
                        gint index);

void etats_prefs_free_all_var (void);

gboolean etats_prefs_left_panel_tree_view_select_last_page (void);

void etats_prefs_onglet_comptes_select_unselect (GtkToggleButton *togglebutton,
                        GtkWidget *tree_view);
void etats_prefs_onglet_categ_budget_check_uncheck_all (GtkToggleButton *togglebutton,
                        GtkWidget *tree_view);
GSList *etats_prefs_onglet_mode_paiement_get_list_rows_selected (const gchar *treeview_name);
void etats_prefs_onglet_mode_paiement_select_rows_from_list (GSList *liste,
                        const gchar *treeview_name);
void etats_prefs_onglet_periode_date_interval_sensitive (gboolean show);

GSList *etats_prefs_tree_view_get_list_rows_selected (const gchar *treeview_name);
gint etats_prefs_tree_view_get_single_row_selected (const gchar *treeview_name);
void etats_prefs_tree_view_select_rows_from_list (GSList *liste,
                        const gchar *treeview_name,
                        gint column);
void etats_prefs_tree_view_select_single_row (const gchar *treeview_name,
                        gint numero);
GtkWidget *etats_prefs_widget_get_widget_by_name (const gchar *parent_name,
                        const gchar *child_name);
gboolean etats_prefs_widget_set_sensitive (const gchar *widget_name,
                        gboolean sensitive);
G_END_DECLS

#endif  /* _ETATS_PREFS_H_ */
