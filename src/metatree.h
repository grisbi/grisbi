#ifndef METATREE_H
#define METATREE_H


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/** Interface between various tree data and metatree */
typedef struct metatree_interface {
    gint depth;
    gint content;	/* content of the metatree : 0 : payee, 1 : category, 2 : budget */
    gint (* tree_currency) ();
    /* Read operations */
    const gchar * meta_name;
    const gchar * meta_name_minus;
    const gchar *meta_sub_name;
    const gchar * no_div_label;
    const gchar * no_sub_div_label;
    gint (* get_without_div_pointer) ();
    gint (* get_div_pointer_from_name) (const gchar *,gboolean);
    gint (* get_sub_div_pointer_from_name) (int,const gchar *,gboolean);
    gint (* div_nb_transactions) (gint);
    gint (* sub_div_nb_transactions) (gint ,gint);
    gchar * (* div_name) (gint);
    gchar * (* sub_div_name) (gint, gint);
    gsb_real (* div_balance) (gint);
    gsb_real (* sub_div_balance) (gint,gint);
    gint (* div_id) (gpointer);
    gint (* sub_div_id) (gpointer);
    GSList * (* div_list) ();
    GSList * (* div_sub_div_list) (gint);
    gint (* div_type) (gint);

    /* Transaction operations */
    gint (* transaction_div_id) (gint);
    gint (* transaction_sub_div_id) (gint);
    gboolean (* transaction_set_div_id) (gint, int);
    gboolean (* transaction_set_sub_div_id) (gint, int);
    gint (* scheduled_div_id) (int);
    gint (* scheduled_sub_div_id) (int);
    gboolean (* scheduled_set_div_id) (int, int);
    gboolean (* scheduled_set_sub_div_id) (int, int);

    /* Write operations on div */
    gint (* add_div) ();
    gint (* add_sub_div) (int);
    gboolean (* remove_div) (int);
    gboolean (* remove_sub_div) (int,int);
    gboolean (* add_transaction_to_div) (gint, int);
    void (* add_transaction_to_sub_div) (gint, int, int);
    void (* remove_transaction_from_div) (int);
    void (* remove_transaction_from_sub_div) (int);

    /* sauvegarde dernière sélection */
    gboolean (* hold_position_set_path) ( GtkTreePath *);
    gboolean (* hold_position_set_expand) ( gboolean );

} MetatreeInterface;


/* structure permettant de mémoriser la dernière sélection dans un metatree */
struct metatree_hold_position
{
    GtkTreePath *path;
    gboolean expand;
};


enum meta_tree_columns {
    META_TREE_TEXT_COLUMN,
    META_TREE_ACCOUNT_COLUMN,
    META_TREE_BALANCE_COLUMN,
    /* this is the number of the current row : either number of categ/budget/payee,
     * either number of sub-categ/sub-budget,
     * either number of transaction */
    META_TREE_POINTER_COLUMN,
    /* number of categ/budget */
    META_TREE_NO_DIV_COLUMN,
    /* number of the sub-categ/sub-budget */
    META_TREE_NO_SUB_DIV_COLUMN,
    /* number of the transaction */
    META_TREE_NO_TRANSACTION_COLUMN,
    META_TREE_FONT_COLUMN,
    META_TREE_XALIGN_COLUMN,
    META_TREE_DATE_COLUMN,
    META_TREE_NUM_COLUMNS,
}; 

enum meta_tree_row_type {
    META_TREE_DIV,
    META_TREE_TRANS_S_S_DIV,
    META_TREE_SUB_DIV,
    META_TREE_TRANSACTION,
    META_TREE_INVALID,
};

#define META_TREE_COLUMN_TYPES G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, \
				 G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, \
				 G_TYPE_INT, G_TYPE_FLOAT, G_TYPE_POINTER


/*START_DECLARATION*/
void appui_sur_ajout_sub_division ( GtkTreeModel * model );
gboolean division_activated ( GtkTreeView * treeview, GtkTreePath * path,
                        GtkTreeViewColumn * col, gpointer userdata );
gboolean division_column_expanded  ( GtkTreeView * treeview, GtkTreeIter * iter, 
                        GtkTreePath * tree_path, gpointer user_data ) ;
gboolean division_drag_data_received ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
                        GtkSelectionData * selection_data );
gboolean division_row_drop_possible ( GtkTreeDragDest * drag_dest, GtkTreePath * dest_path,
                        GtkSelectionData * selection_data );
void expand_arbre_division ( GtkWidget *bouton, gint depth );
void fill_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
                        GtkTreeIter * iter, gint division );
void fill_sub_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
                        GtkTreeIter * iter,
                        gint division,
                        gint sub_division );
GtkTreeIter *get_iter_from_div ( GtkTreeModel * model, int div, int sub_div );
gint metatree_get_nbre_transactions_sans_sub_div ( GtkWidget *tree_view );
enum meta_tree_row_type metatree_get_row_type_from_tree_view ( GtkWidget *tree_view );
void metatree_manage_sub_divisions ( GtkWidget *tree_view );
void metatree_new_division ( GtkTreeModel * model );
void metatree_register_widget_as_linked ( GtkTreeModel * model,
                        GtkWidget * widget,
                        gchar * link_type );
void metatree_remove_transaction ( GtkTreeView * tree_view,
                        MetatreeInterface * iface, 
                        gint transaction,
                        gboolean delete_transaction );
gboolean metatree_selection_changed ( GtkTreeSelection * selection, GtkTreeModel * model );
void metatree_set_linked_widgets_sensitive ( GtkTreeModel * model,
                        gboolean sensitive,
                        gchar * link_type );
gboolean metatree_sort_column  ( GtkTreeModel * model,
                        GtkTreeIter * a, GtkTreeIter * b,
                        gpointer user_data );
gboolean supprimer_division ( GtkTreeView * tree_view );
void update_transaction_in_tree ( MetatreeInterface * iface, 
                        GtkTreeModel * model, 
                        gint transaction_number );
/*END_DECLARATION*/

#endif
