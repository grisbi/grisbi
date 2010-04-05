#ifndef _GSB_FYEAR_H
#define _GSB_FYEAR_H (1)

enum fyear_list_columns {
    FYEAR_COL_NAME = 0,
    FYEAR_COL_NUMBER,
    FYEAR_COL_VIEW,
};


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_fyear_get_fyear_from_combobox ( GtkWidget *combo_box,
                        const GDate *date );
gboolean gsb_fyear_hide_iter_by_name ( GtkTreeModel *model, gchar *name );
void gsb_fyear_init_variables ( void );
GtkWidget *gsb_fyear_make_combobox ( gboolean set_automatic );
GtkWidget *gsb_fyear_make_combobox_new ( GtkTreeModel *model,
                        gboolean set_automatic );
gboolean gsb_fyear_select_iter_by_number ( GtkWidget *combo_box,
                        GtkTreeModel *model,
                        GtkTreeModel *model_filter,
                        gint fyear_number );
gboolean gsb_fyear_set_automatic ( gboolean set_automatic );
gboolean gsb_fyear_set_combobox_history ( GtkWidget *combo_box,
                        gint fyear_number );
gboolean gsb_fyear_update_fyear_list ( void );
gboolean gsb_fyear_update_fyear_list_new ( GtkTreeModel *model,
                        GtkTreeModel *model_filter,
                        gchar *title );
/* END_DECLARATION */
#endif
