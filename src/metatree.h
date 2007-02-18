#ifndef METATREE_H
#define METATREE_H


/* START_INCLUDE_H */
#include "./metatree.h"
#include "./include.h"
/* END_INCLUDE_H */


/** Interface between various tree data and metatree */
typedef struct metatree_interface {
    gint depth;
    gint (* tree_currency) ();
    /* Read operations */
    const gchar * meta_name;
    const gchar * no_div_label;
    const gchar * no_sub_div_label;
    gpointer (* get_without_div_pointer) ();
    gpointer (* get_div_pointer) (int);
    gpointer (* get_sub_div_pointer) (int,int);
    gpointer (* get_div_pointer_from_name) (const gchar *,gboolean);
    gpointer (* get_sub_div_pointer_from_name) (int,const gchar *,gboolean);
    gint (* div_nb_transactions) (gpointer);
    gint (* sub_div_nb_transactions) (gpointer,gpointer);
    const gchar * (* div_name) (gpointer);
    const gchar * (* sub_div_name) (gpointer);
    gsb_real (* div_balance) (gpointer);
    gsb_real (* sub_div_balance) (gpointer,gpointer);
    gint (* div_id) (gpointer);
    gint (* sub_div_id) (gpointer);
    GSList * (* div_list) ();
    GSList * (* div_sub_div_list) (gpointer);
    gint (* div_type) (gpointer);

    /* Transaction operations */
    gint (* transaction_div_id) (gpointer);
    gint (* transaction_sub_div_id) (gpointer);
    void (* transaction_set_div_id) (gpointer, int);
    void (* transaction_set_sub_div_id) (gpointer, int);
    gint (* scheduled_div_id) (int);
    gint (* scheduled_sub_div_id) (int);
    void (* scheduled_set_div_id) (int, int);
    void (* scheduled_set_sub_div_id) (int, int);

    /* Write operations on div */
    gint (* add_div) ();
    gint (* add_sub_div) (int);
    gboolean (* remove_div) (int);
    gboolean (* remove_sub_div) (int,int);
    gboolean (* add_transaction_to_div) (gpointer, int);
    gboolean (* add_transaction_to_sub_div) (gpointer, int, int);
    gboolean (* remove_transaction_from_div) (gpointer, int);
    gboolean (* remove_transaction_from_sub_div) (gpointer, int, int);

} MetatreeInterface;

enum meta_tree_columns {
    META_TREE_TEXT_COLUMN,
    META_TREE_ACCOUNT_COLUMN,
    META_TREE_BALANCE_COLUMN,
    META_TREE_POINTER_COLUMN,
    META_TREE_NO_DIV_COLUMN,
    META_TREE_NO_SUB_DIV_COLUMN,
    META_TREE_FONT_COLUMN,
    META_TREE_XALIGN_COLUMN,
    META_TREE_DATE_COLUMN,
    META_TREE_NUM_COLUMNS,
}; 

enum meta_tree_row_type {
    META_TREE_DIV,
    META_TREE_SUB_DIV,
    META_TREE_TRANSACTION,
    META_TREE_INVALID,
};

#define META_TREE_COLUMN_TYPES G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, \
				 G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT, \
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
			 GtkTreeIter * iter, gpointer division );
void fill_sub_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			     GtkTreeIter * iter, gpointer division,
			     gpointer sub_division );
GtkTreeIter * get_iter_from_div ( GtkTreeModel * model, int div, int sub_div );
void metatree_new_division ( GtkTreeModel * model );
void metatree_register_widget_as_linked ( GtkTreeModel * model, GtkWidget * widget,
					  gchar * link_type );
gboolean metatree_selection_changed ( GtkTreeSelection * selection, GtkTreeModel * model );
void metatree_set_linked_widgets_sensitive ( GtkTreeModel * model, gboolean sensitive,
					     gchar * link_type );
inline gboolean metatree_sort_column  ( GtkTreeModel * model, 
					GtkTreeIter * a, GtkTreeIter * b, 
					gpointer user_data );
gboolean supprimer_division ( GtkTreeView * tree_view );
void update_transaction_in_tree ( MetatreeInterface * iface, GtkTreeModel * model, 
				  gint transaction_number );
/*END_DECLARATION*/

#endif
